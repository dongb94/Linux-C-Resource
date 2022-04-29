/**
 * @file EventTimer.cpp
 * @author donggeon byeon (dongb94@gmail.com)
 * @version 1.12
 * @date 2022-04-29
 * 
 */

#include "EventTimer.h"

#define GET_HASH_MEM(smHandle, key, pData) (Get_hashed_shm(&smHandle, key, (void**)&pData))
#define NEW_HASH_MEM(smHandle, key, pData) (New_hashed_shm(&smHandle, key, (void**)&pData))

bool EventTimerWheel::flag;
USHORT EventTimerWheel::eventSerial;
st_HashedShmHandle EventTimerWheel::m_shmTimerEvent;
UINT32 *EventTimerWheel::hourHead[TIMER_WHEEL_SIZE], *EventTimerWheel::minuteHead[TIMER_WHEEL_SIZE], *EventTimerWheel::secondHead[TIMER_WHEEL_SIZE];
UINT32 *EventTimerWheel::hourPointer, *EventTimerWheel::minutePointer, *EventTimerWheel::secondPointer;
UINT32 EventTimerWheel::minuteCurrent, EventTimerWheel::secondCurrent;
EventStruct *EventTimerWheel::selectedEvent;

static struct TimerRemoteControler _timer_rm;
struct TimerRemoteControler *timer;

EventTimerWheel::EventTimerWheel()
{
	timer = &_timer_rm;
	timer->addTimeOut = AddTimeOut;
	timer->addTimeOutDelay = AddTimeOut;
	timer->tick = Tick;
	timer->removeUVS = RemoveUVSEvent;
	timer->remove = RemoveEvent;
	timer->getEvent = GetEvent;
	timer->getCurrentTimeFormat = GetCurrentTimeFormat;
	timer->getCurrentTimeTM = GetCurrentTimeTM;
	timer->getCurrentSecondFromTime = GetCurrentSecondFromTime;
	timer->getMillisecondFromCurrentTime = GetMillisecondFromCurrentTime;
	timer->getMillisecondFromCurrentTime2 = GetMillisecondFromCurrentTime;
	timer->getDayAfterValue = GetDayAfterValue;
	timer->getDayAfterResetType = GetDayAfterResetType;
	timer->getLastDayofMonth = GetLastDayofMonth;

	flag = false;
	eventSerial = 1;

	InitSharedMemory();

	if(!m_start)
	{
		std::thread t([=]() 
		{
			struct timeb m_time;
			ftime(&m_time);
			struct tm *tm_ptr;
			tm_ptr = localtime(&m_time.time);
			printf("\tTimer Start Time : %d:%d:%d\n", tm_ptr->tm_hour, tm_ptr->tm_min, tm_ptr->tm_sec);

			minuteCurrent = (UINT32)tm_ptr->tm_min;
			secondCurrent = (UINT32)(tm_ptr->tm_sec * (TIMER_WHEEL_TICK_PER_SECOND));
			secondCurrent += m_time.millitm * TIMER_WHEEL_TICK_PER_SECOND / 1000;

			int lastTick = m_time.millitm;

			m_start = true;

			int clockTime;
			while(true)
			{
				ftime(&m_time);
				clockTime = m_time.millitm;
				// dAppLog(LOG_DEBUG, "[clock %d]-[lastTick %d]=[%d]", clockTime, lastTick, clockTime - lastTick);
				// dAppLog(LOG_DEBUG, "Delay %d > %d [TIME : %d]", clockTime - lastTick, CLOCKS_PER_SEC, clockTime/CLOCKS_PER_SEC);
				// printf("[clock %d]-[lastTick %d]=[%d]\n", clockTime, lastTick, clockTime - lastTick);
				if(clockTime < lastTick) clockTime += 1000;
				if(clockTime-lastTick >= TIMER_WHEEL_TICK_TIME)
				{
					// dAppLog(LOG_DEBUG, "[clock %d]-[tick %d]=[%d]", clockTime, tick, clockTime - tick);
					lastTick = (lastTick+TIMER_WHEEL_TICK_TIME) % 1000;
					if(timer->tick()<0)
					{
						m_start = false;
						return;
					}
				}

				usleep(10000);
			}
		});
		t.detach();
	}
	while (!m_start)
	{
		usleep(1);
	}
}

EventTimerWheel::~EventTimerWheel(){}

int EventTimerWheel::InitSharedMemory()
{
	int res;
	UINT64 *headKey;
	UINT64 key;
	flag = false;

	dAppLog(LOG_DEBUG, "Init Event Timer SharedMemory");

	Init_hashed_shm_handle(&m_shmTimerEvent, TIMER_WHEEL_EVENT_SIZE, sizeof(EventStruct), TIMER_WHEEL_EVENT_SIZE, S_SSHM_EVENT_TIMEWHEEL);
	res = Init_hashed_shm(&m_shmTimerEvent);
	if(res < 0)
	{
		dAppLog(LOG_DEBUG, "Init Event Timer SharedMemory Error [res : %d]\n", res);
		return res;
	}

	Init_hashed_shm_handle(&m_smTimerWheelHour, TIMER_WHEEL_SIZE + 1, sizeof(UINT32), TIMER_WHEEL_SIZE + 1, S_SSHM_EVENT_TIMEWHEEL_HOUR);
	res = Init_hashed_shm(&m_smTimerWheelHour);
	if(res < 0)
	{
		dAppLog(LOG_DEBUG, "Init Event Timer(HOUR) SharedMemory Error [res : %d]\n", res);
		return res;
	}
	for(int i=0; i<TIMER_WHEEL_SIZE; i++)
	{
		key = GET_TIMER_KEY(i);
		res = Get_hashed_shm(&m_smTimerWheelHour, key, (void**)&(hourHead[i]));
		if(res < 0){
			res = New_hashed_shm(&m_smTimerWheelHour, key, (void**)&(hourHead[i]));
			if(res < 0)
			{
				dAppLog(LOG_DEBUG, "New Hash shm Error (Hour) [res : %d][ullKey : %llx]", res, key);
				return -1;
			}
		}
	}
	key = GET_TIMER_KEY(TIMER_WHEEL_SIZE);
	res = Get_hashed_shm(&m_smTimerWheelHour, key, (void**)&hourPointer);
	if(res < 0){
		res = New_hashed_shm(&m_smTimerWheelHour, key, (void**)&hourPointer);
		if(res < 0)
		{
			dAppLog(LOG_DEBUG, "New Hash shm Error (Hour) [res : %d][ullKey : %llx]", res, key);
			return -1;
		}
	}

	Init_hashed_shm_handle(&m_smTimerWheelMinute, TIMER_WHEEL_SIZE + 1, sizeof(UINT32), TIMER_WHEEL_SIZE + 1, S_SSHM_EVENT_TIMEWHEEL_MINUTE);
	res = Init_hashed_shm(&m_smTimerWheelMinute);
	if(res < 0)
	{
		dAppLog(LOG_DEBUG, "Init Event Timer(MINUTE) SharedMemory Error [res : %d]\n", res);
		return res;
	}
	for(int i=0; i<TIMER_WHEEL_SIZE; i++)
	{
		key = GET_TIMER_KEY(i);
		res = Get_hashed_shm(&m_smTimerWheelMinute, key, (void**)&(minuteHead[i]));
		if(res < 0){
			res = New_hashed_shm(&m_smTimerWheelMinute, key, (void**)&(minuteHead[i]));
			if(res < 0)
			{
				dAppLog(LOG_DEBUG, "New Hash shm Error (MINUTE) [res : %d][ullKey : %llx]", res, key);
				return -1;
			}
		}
	}
	key = GET_TIMER_KEY(TIMER_WHEEL_SIZE);
	res = Get_hashed_shm(&m_smTimerWheelMinute, key, (void**)&minutePointer);
	if(res < 0){
		res = New_hashed_shm(&m_smTimerWheelMinute, key, (void**)&minutePointer);
		if(res < 0)
		{
			dAppLog(LOG_DEBUG, "New Hash shm Error (MINUTE) [res : %d][ullKey : %llx]", res, key);
			return -1;
		}
	}

	Init_hashed_shm_handle(&m_smTimerWheelSecond, TIMER_WHEEL_SIZE + 1, sizeof(UINT32), TIMER_WHEEL_SIZE + 1, S_SSHM_EVENT_TIMEWHEEL_SECOND);
	res = Init_hashed_shm(&m_smTimerWheelSecond);
	if(res < 0)
	{
		dAppLog(LOG_DEBUG, "Init Event Timer(SECOND) SharedMemory Error [res : %d]\n", res);
		return res;
	}
	for(int i=0; i<TIMER_WHEEL_SIZE; i++)
	{
		key = GET_TIMER_KEY(i);
		res = Get_hashed_shm(&m_smTimerWheelSecond, key, (void**)&(secondHead[i]));
		if(res < 0){
			res = New_hashed_shm(&m_smTimerWheelSecond, key, (void**)&(secondHead[i]));
			if(res < 0)
			{
				dAppLog(LOG_DEBUG, "New Hash shm Error (SECOND) [res : %d][ullKey : %llx]", res, key);
				return -1;
			}
		}
	}
	key = GET_TIMER_KEY(TIMER_WHEEL_SIZE);
	res = Get_hashed_shm(&m_smTimerWheelSecond, key, (void**)&secondPointer);
	if(res < 0){
		res = New_hashed_shm(&m_smTimerWheelSecond, key, (void**)&secondPointer);
		if(res < 0)
		{
			dAppLog(LOG_DEBUG, "New Hash shm Error (SECOND) [res : %d][ullKey : %llx]", res, key);
			return -1;
		}
	}

	dAppLog(LOG_DEBUG, "Init Event Timer SharedMemory End");
}

/**
* @return 즉시 실행 = 0, 실패 < 0, 성공 tid 
*/
int EventTimerWheel::AddTimeOut(uint64 functionId, uint64 millisecond, uint64 eventVar, bool repeat, int repeatCount, uint64 tid, bool overriding)
{
	return AddTimeOut(functionId, millisecond, eventVar, repeat, repeatCount, millisecond, tid, overriding);
}

/**
 * @brief 실행 대기 시간과 반복 주기가 다를경우 사용
 * @return 즉시 실행 = 0, 실패 < 0, 성공 tid 
 */
int EventTimerWheel::AddTimeOut(uint64 functionId, uint64 startTime, uint64 eventVar, bool repeat, int repeatCount, uint64 repeatDelay, uint64 tid, bool overriding)
{
	int res;
	EventStruct* newEvent;

	if(tid == 0)
	{
		tid = eventSerial + FIXED_EVENT_SIZE;
		eventSerial = (eventSerial+1) % (TIMER_WHEEL_EVENT_SIZE-FIXED_EVENT_SIZE);
	}

	res = GetEvent(tid, &newEvent);
	if(res < 0)
	{
		dAppLog(LOG_CRI, "Get Timer Event Error [tid : %lld]", tid);
		return -1;
	}

	while(newEvent->tid != 0)
	{
		if(overriding) 
		{
			RemoveEvent(newEvent->tid);
			break;
		}
		tid = (tid + 1) % TIMER_WHEEL_EVENT_SIZE;
		if(tid == 0) tid = FIXED_EVENT_SIZE;

		res = GetEvent(tid, &newEvent);
		if(res < 0)
		{
			dAppLog(LOG_CRI, "Get Timer Event Error [tid : %lld]", tid);
			return -1;
		}
	}

	newEvent->preEventKey	= 0;
	newEvent->nextEventKey	= 0;
	newEvent->tid = tid;
	newEvent->millisecond = repeatDelay;
	newEvent->functionId = functionId;
	newEvent->eventVar = eventVar;
	newEvent->triggerTime = time(NULL) + startTime;
	newEvent->repeat = repeat;
	newEvent->repeatCount = repeatCount;

	if(startTime == 0)
	{
		dAppLog(LOG_DEBUG, "startTime == 0 [tid %lld][functionID %lld][EventVar %lld]", newEvent->tid, newEvent->functionId, newEvent->eventVar);
		RunEvent(newEvent);
		if(newEvent->repeat && repeatDelay !=0)
		{
			if(newEvent->repeatCount < 0)	// infinity repeat
				AddTimeOut(newEvent->functionId, newEvent->millisecond, newEvent->eventVar, true, -1, newEvent->tid, true);
			else if(newEvent->repeatCount > 0)
				AddTimeOut(newEvent->functionId, newEvent->millisecond, newEvent->eventVar, true, newEvent->repeatCount-1, newEvent->tid, true);
		}
		else
		{
			memset(newEvent, 0, sizeof(EventStruct));
		}
		return 0;
	}

	
	while(flag){usleep(1000);}
	flag = true;

	int hour = startTime / MILLISECOND_PER_HOUR;
	int minute = startTime / MILLISECOND_PER_MINUTE;
	UINT32 *HeadKeyPointer = NULL;

	if(hour != 0){
		int hourSlot = ((hour-1) + *hourPointer) & 0b111;		// == hour % 8

		HeadKeyPointer = hourHead[hourSlot];
		if(HeadKeyPointer == NULL)
		{
			dAppLog(LOG_CRI, "Get Hour Event Shared Memory Error [hourslot : %d][key : %llx]", hourSlot, GET_TIMER_KEY(hourSlot));
			flag = false;
			return -3;
		}

		newEvent->leftCount = (hour-1) >> 3;	 // == hour / 8
		newEvent->leftMilliSecond = (startTime % MILLISECOND_PER_HOUR) + (minuteCurrent * MILLISECOND_PER_MINUTE) + (secondCurrent * MILLISECOND / TIMER_WHEEL_TICK_PER_SECOND);
	}
	else if(minute != 0){
		int minuteSlot = ((minute-1) + *minutePointer) & 0b111;

		HeadKeyPointer = minuteHead[minuteSlot];
		if(HeadKeyPointer == NULL)
		{
			dAppLog(LOG_CRI, "Get Minute Event Shared Memory Error [minuteSlot : %d][key : %llx]", minuteSlot, GET_TIMER_KEY(minuteSlot));
			flag = false;
			return -3;
		}

		newEvent->leftCount = (minute-1) >> 3;
		newEvent->leftMilliSecond = (startTime % MILLISECOND_PER_MINUTE) + (secondCurrent * MILLISECOND / TIMER_WHEEL_TICK_PER_SECOND);

		// dAppLog(LOG_DEBUG, " Add Time Tick [tid %lld][startTime %lld][fid %lld][MinuteSlot %d][leftCount %d][secondPointer %d]", tid, startTime, newEvent->functionId, minuteSlot, newEvent->leftCount, *secondPointer);
	}
	else{
		int timeTick = startTime * TIMER_WHEEL_TICK_PER_SECOND / MILLISECOND;
		int secondSlot = (timeTick + *secondPointer) & 0b111;

		HeadKeyPointer = secondHead[secondSlot];
		if(HeadKeyPointer == NULL)
		{
			dAppLog(LOG_CRI, "Get Second Event Shared Memory Error [secondSlot : %d][key : %llx]", secondSlot, GET_TIMER_KEY(secondSlot));
			flag = false;
			return -3;
		}

		newEvent->leftCount = timeTick >> 3;
		newEvent->leftMilliSecond = startTime % (MILLISECOND / TIMER_WHEEL_TICK_PER_SECOND);

		// dAppLog(LOG_DEBUG, " Add Time Tick [tid %lld][startTime %lld][fid %lld][timeTick %d][secondSlot %d][leftCount %d][secondPointer %d]", tid, startTime, newEvent->functionId, timeTick, secondSlot, newEvent->leftCount, *secondPointer);
	}

	if(HeadKeyPointer == NULL)
	{
		dAppLog(LOG_CRI, "[[HeadKeyPointer was NULL]]");
		flag = false;
		return -2;
	}

	if((*HeadKeyPointer) == 0)
	{
		*HeadKeyPointer = tid;
	}
	else
	{
		res = GetEvent(*HeadKeyPointer, &selectedEvent);
		if(res < 0)
		{
			dAppLog(LOG_CRI, "Get Timer Event Error (Add Time out event) [tid : %lld]", *HeadKeyPointer);
			flag = false;
			return -1;
		}

		newEvent->nextEventKey = *HeadKeyPointer;
		selectedEvent->preEventKey = tid;
		*HeadKeyPointer = tid;
	}

	// printf("Add New Event [tid : %d]\n", newEvent->tid);
	dAppLog(LOG_DEBUG, "Add New Event [pre %5lld][next %5lld][tid %5lld][startTime %9lld][function %5lld][var1 %lld][repeat %d][repeat count %d][repeat delay %lld][%d:%d][leftCount : %d][leftMillisecond : %d]", 
						newEvent->preEventKey, newEvent->nextEventKey, newEvent->tid, startTime, newEvent->functionId, eventVar, newEvent->repeat, newEvent->repeatCount, newEvent->millisecond,
						hour, minute, newEvent->leftCount, newEvent->leftMilliSecond);

	flag = false;

	return newEvent->tid;
}

int EventTimerWheel::RemoveUVSEvent(uint64 tid)
{
	if(tid < 1 || tid >= TIMER_WHEEL_EVENT_SIZE) return 0;

	EventStruct *st_event;
	GetEvent(tid, &st_event);

	if(st_event == NULL)
	{
		return -1;
	}

	CUniversalVariableSpace* UVS = (CUniversalVariableSpace*)smUniversalVariableSpace[0];
	UVS->RemoveData(st_event->eventVar);

	RemoveEvent(tid);

	return 0;
}

int EventTimerWheel::RemoveEvent(uint64 tid)
{
	if(tid < 1 || tid >= TIMER_WHEEL_EVENT_SIZE) return 0;

	while(flag){usleep(1000);}
	flag = true;

	EventStruct *st_event, *st_preEvent, *st_nextEvent;
	GetEvent(tid, &st_event);

	if(st_event == NULL)
	{
		flag = false;
		return -1;
	}

	// dAppLog(LOG_DEBUG, "Remove Event [tid %lld][var %lld]", tid, st_event->eventVar);

	// 원소 제거
	if(st_event->preEventKey != 0)
	{
		GetEvent(st_event->preEventKey, &st_preEvent);
		st_preEvent->nextEventKey = st_event->nextEventKey;
	}
	else // 헤더인 경우
	{
		for(int i = 0; i<TIMER_WHEEL_SIZE; i++)
		{
			if(*(hourHead[i]) == tid)
			{
				*(hourHead[i]) = st_event->nextEventKey;
				break;
			}
			else if(*(minuteHead[i]) == tid)
			{
				*(minuteHead[i]) = st_event->nextEventKey;
				break;
			}
			else if(*(secondHead[i]) == tid)
			{
				*(secondHead[i]) = st_event->nextEventKey;
				break;
			}
		}
	}

	if(st_event->nextEventKey != 0)
	{
		GetEvent(st_event->nextEventKey, &st_nextEvent);
		st_nextEvent->preEventKey = st_event->preEventKey;
	}

	memset(st_event, 0, sizeof(EventStruct));

	flag = false;

	return 0;
}

int EventTimerWheel::Tick()
{
	int res;
	INT64 key;
	UINT32 HeadKeyPointer;

	EventStruct *pst_event;
	EventStruct	*pst_preEvent, *pst_nextEvent;

	EventStruct st_eventData;

	if(secondPointer == NULL) return 0;

	while(flag){usleep(1000);}
	flag = true;

	// printf("tick [%d:%02d:%04d]\n", *hourPointer, minuteCurrent, secondCurrent);
	HeadKeyPointer = *(secondHead[*secondPointer]);
	// printf("%d\n", HeadKeyPointer);
	while(HeadKeyPointer != 0)
	{
		GetEvent(HeadKeyPointer, &pst_event);
		// dAppLog(LOG_DEBUG, "Event Tick SECOND[TID %lld][leftCount %d]", st_event->tid, st_event->leftCount);
		if(pst_event->tid == 0)
		{
			dAppLog(LOG_CRI, "event error [%d]", HeadKeyPointer);
			flag = false;
			return -1;
		}
		else if(pst_event->leftCount <=0)
		{			
			// 원소 제거
			if(*(secondHead[*secondPointer]) == pst_event->tid)
			{
				*(secondHead[*secondPointer]) = pst_event->nextEventKey;
			}
			HeadKeyPointer = pst_event->nextEventKey;

			if(pst_event->preEventKey != 0)
			{
				GetEvent(pst_event->preEventKey, &pst_preEvent);
				pst_preEvent->nextEventKey = pst_event->nextEventKey;
			}
			if(pst_event->nextEventKey != 0)
			{
				GetEvent(pst_event->nextEventKey, &pst_nextEvent);
				pst_nextEvent->preEventKey = pst_event->preEventKey;
			}

			st_eventData = *pst_event; // 값 복사
			if(pst_event->repeat)
			{
				dAppLog(LOG_DEBUG, "[Tick] [functionID %lld][Second %lld][EventVar %lld][Repeat %d][Tid %lld]", pst_event->functionId, pst_event->millisecond, pst_event->eventVar, pst_event->repeat, pst_event->tid);
				flag = false;
				if(pst_event->repeatCount < 0)	// infinity repeat
					AddTimeOut(pst_event->functionId, pst_event->millisecond, pst_event->eventVar, true, -1, pst_event->tid, true);
				else if(pst_event->repeatCount > 0)
					AddTimeOut(pst_event->functionId, pst_event->millisecond, pst_event->eventVar, true, pst_event->repeatCount-1, pst_event->tid, true);
				while(flag){usleep(1000);}
				flag = true;
			}
			else{
				memset(pst_event, 0, sizeof(EventStruct));
			}

			flag = false;
			RunEvent(&st_eventData);
			while(flag){usleep(1000);}
			flag = true;

			continue;
		}
		else
		{
			// printf("[TID %lld][Tick %d][left %d]\n", st_event->tid, *secondPointer, st_event->leftCount);
			pst_event->leftCount--;
		}

		HeadKeyPointer = pst_event->nextEventKey;
		//printf("%d\n", HeadKeyPointer);
	}

	*secondPointer = (*secondPointer + 1) & 0b111;
	secondCurrent = (secondCurrent + 1) % (60 * TIMER_WHEEL_TICK_PER_SECOND);
	if(secondCurrent != 0){
		flag = false;
		return 0;
	}

	// printf("tick minute [%d:%02d:%04d] minute pointer [%d]\n", *hourPointer, minuteCurrent, secondCurrent, *minutePointer);
	HeadKeyPointer = *(minuteHead[*minutePointer]);
	while(HeadKeyPointer != 0)
	{
		GetEvent(HeadKeyPointer, &pst_event);
		HeadKeyPointer = pst_event->nextEventKey; // 내리면 안됨.
		// dAppLog(LOG_DEBUG, "Event Tick MINITE[TID %lld][leftCount %d]", st_event->tid, st_event->leftCount);
		if(pst_event->leftCount <=0)
		{
			
			// 원소 제거
			if(pst_event->preEventKey != 0)
			{
				GetEvent(pst_event->preEventKey, &pst_preEvent);
				pst_preEvent->nextEventKey = pst_event->nextEventKey;
			}
			if(pst_event->nextEventKey != 0)
			{
				GetEvent(pst_event->nextEventKey, &pst_nextEvent);
				pst_nextEvent->preEventKey = pst_event->preEventKey;
			}

			// 헤드 노드 갱신
			if(*(minuteHead[*minutePointer]) == pst_event->tid)
			{
				*(minuteHead[*minutePointer]) = pst_event->nextEventKey;
			}

			// 타이머 휠 갱신
			flag = false;
			AddTimeOut(pst_event->functionId, pst_event->leftMilliSecond, pst_event->eventVar, pst_event->repeat, pst_event->repeatCount, pst_event->millisecond, pst_event->tid, true);
			while(flag){usleep(1000);}
			flag = true;
		}
		else
		{
			pst_event->leftCount--;
		}
	}

	*minutePointer = (*minutePointer + 1) & 0b111;
	minuteCurrent = (minuteCurrent + 1) % 60;
	if(minuteCurrent != 0) 
	{
		flag = false;
		return 1;
	}

	// printf("tick hour [%d:%02d:%04d]\n", *hourPointer, minuteCurrent, secondCurrent);
	HeadKeyPointer = *(hourHead[*hourPointer]);
	while(HeadKeyPointer != 0)
	{
		GetEvent(HeadKeyPointer, &pst_event);
		HeadKeyPointer = pst_event->nextEventKey;
		// dAppLog(LOG_DEBUG, "Event Tick HOUR[TID %lld][leftCount %d]", st_event->tid, st_event->leftCount);
		if(pst_event->leftCount <=0)
		{
			// 원소 제거
			if(pst_event->preEventKey != 0)
			{
				GetEvent(pst_event->preEventKey, &pst_preEvent);
				pst_preEvent->nextEventKey = pst_event->nextEventKey;
			}
			if(pst_event->nextEventKey != 0)
			{
				GetEvent(pst_event->nextEventKey, &pst_nextEvent);
				pst_nextEvent->preEventKey = pst_event->preEventKey;
			}

			// 헤드 노드 갱신
			if(*(hourHead[*hourPointer]) == pst_event->tid)
			{
				*(hourHead[*hourPointer]) = pst_event->nextEventKey;
			}

			// 타이머 휠 갱신
			flag = false;
			AddTimeOut(pst_event->functionId, pst_event->leftMilliSecond, pst_event->eventVar, pst_event->repeat, pst_event->repeatCount, pst_event->millisecond, pst_event->tid, true);
			while(flag){usleep(1000);}
			flag = true;

		}
		else
		{
			pst_event->leftCount--;
		}

	}

	*hourPointer = (*hourPointer + 1) & 0b111;

	flag = false;
	return 2;
}

int EventTimerWheel::RunEvent(EventStruct *st_evnet)
{
	// dAppLog(LOG_DEBUG, "[RunEvent] Event Run [tid : %lld][functionId : %lld][EventVar : %lld]", st_evnet->tid, st_evnet->functionId, st_evnet->eventVar);

	switch (st_evnet->functionId)
	{
	case TIMER_FUNCTION_DISCONNECT_USER_LOGOUT :
		DisconnectLogout(st_evnet->eventVar);
		break;

	case TIMER_FUNCTION_EVENT_BOSS_MONSTER:
		EventBossMonster(st_evnet->eventVar);
		break;

	case TIMER_FUNCTION_CHARACTER_BUFF_END:
		EndBuff(st_evnet->eventVar, st_evnet->tid);
		break;

	case TIMER_FUNCTION_ID_ADD_HP:
		AddHp(st_evnet->eventVar);
		break;

	case TIMER_FUNCTION_DUNGEON_CLEAR_TIME_OVER:
	{
		DungeonTimeOut(st_evnet->eventVar);
	}	break;

	case TIMER_FUNCTION_DUNGEON_USE_TIME_END:
	{
		FeildDungeonTimeOut(st_evnet->eventVar);
	}	break;

	case TIMER_FUNCTION_EXP_DOUBLE_BUFF_END:
		ExpDoubleBuffEnd(st_evnet->eventVar);
		break;

	case TIMER_FUNCTION_RESET_DAILY:
		ResetEventTimer(st_evnet->eventVar);
		break;

	case TIMER_FUNCTION_RESET_WEEKLY:
		ResetEventTimer(st_evnet->eventVar);
		break;

	case TIMER_FUNCTION_RESET_MONTHLY:
		ResetEventTimer(st_evnet->eventVar);
		break;

	case TIMER_FINCTION_RESET_SHOP_FIXEDTIME_PURCHASE:
		ResetEventTimer(st_evnet->eventVar);
		break;

	default:
		break;
	}

	return 0;
}

int EventTimerWheel::GetEvent(uint64 tid, EventStruct **event)
{
	int res;
	tid = GET_HASH_KEY(tid);
	res = Get_hashed_shm(&m_shmTimerEvent, tid, (void**)event);
	if(res < 0)
	{
		res = New_hashed_shm(&m_shmTimerEvent, tid, (void**)event);
		if(res < 0)
		{
			dAppLog(LOG_DEBUG, "New Event Hash shm Error [res : %d][eventKey : %llx]", res, tid);
			return -1;
		}
	}

	return 0;
}

UINT64 EventTimerWheel::GetTriggerTime(uint64 tid)
{
	if(tid < 0) return 0;
	EventStruct *event;
	GetEvent(tid, &event);
	if(event == NULL) return 0;
	return event->triggerTime;
}

UINT64 EventTimerWheel::GetLeftTime(uint64 tid)
{
	if(tid < 0) return 0;
	EventStruct *event;
	GetEvent(tid, &event);
	if(event == NULL) return 0;
	return event->triggerTime - time(NULL);
}

UINT64 EventTimerWheel::GetCurrentTimeFormat()
{
	struct timeb m_time;
	ftime(&m_time);
	struct tm *tm;
	tm = localtime(&m_time.time);

	return (tm->tm_year-100) * 10000000000 + (tm->tm_mon + 1) * 100000000 + tm->tm_mday * 1000000 + tm->tm_hour * 10000 + tm->tm_min * 100 + tm->tm_sec;
}

tm* EventTimerWheel::GetCurrentTimeTM()
{
	struct timeb m_time;
	ftime(&m_time);
	struct tm *tm;
	tm = localtime(&m_time.time);

	return tm;
}

UINT64 EventTimerWheel::GetCurrentSecondFromTime()
{
	struct timeb m_time;
	ftime(&m_time);
	struct tm *tm;
	tm = localtime(&m_time.time);

	return tm->tm_hour * 3600 + tm->tm_min * 60 + tm->tm_sec;
}

UINT64 EventTimerWheel::GetMillisecondFromCurrentTime(tm *time)
{
	struct timeb m_time;
	ftime(&m_time);
	struct tm *tm;
	tm = localtime(&m_time.time);

	int day = (time->tm_year * 365 + time->tm_yday) - (tm->tm_year * 365 + tm->tm_yday) + ((time->tm_year-1)/4 - (time->tm_year-1)/100 + (time->tm_year-1)/400) - ((tm->tm_year-1)/4 - (tm->tm_year-1)/100 + (tm->tm_year-1)/400);
	int second = (time->tm_hour * 3600 + time->tm_min * 60 + time->tm_sec) - (tm->tm_hour * 3600 + tm->tm_min * 60 + tm->tm_sec);
	UINT64 millisecond = day * 86400000 + second * 1000 - m_time.millitm;

	return millisecond;
}

UINT64 EventTimerWheel::GetMillisecondFromCurrentTime(time_t *time)
{
	struct timeb m_time;
	ftime(&m_time);

	UINT64 millisecond = (*time - m_time.time) * 1000 - m_time.millitm;

	return millisecond;
}

int EventTimerWheel::GetLastDayofMonth(int Year, int Month)
{
	if(Month == 2 && CHECK_LEAP_YEAR(Year))
	{
		return 29;
	}
	
	return days[Month];
}

UINT64 EventTimerWheel::GetDayAfterValue(UCHAR value)
{
	struct timeb m_time;
	ftime(&m_time);
	struct tm *tm;
	tm = localtime(&m_time.time);

	if(CHECK_LEAP_YEAR(tm->tm_year))
	{
		days[1] = 29;
	}
	else
	{
		days[1] = 28;
	}

	tm->tm_mday += value;

	if(tm->tm_mday > days[tm->tm_mon])
	{
		int overDay = days[tm->tm_mon] - tm->tm_mday;
		tm->tm_mday = overDay * -1;
		tm->tm_mon++;

		if(tm->tm_mon == 12)
		{
			tm->tm_mon = 0;
			tm->tm_year++;
		}
	}

	return (tm->tm_year-100) * 10000000000 + (tm->tm_mon + 1) * 100000000 + tm->tm_mday * 1000000 + tm->tm_hour * 10000 + tm->tm_min * 100 + tm->tm_sec;
}

time_t EventTimerWheel::GetDayAfterResetType(UCHAR ResetType)
{
	struct timeb m_time;
	ftime(&m_time);
	struct tm *tm;
	tm = localtime(&m_time.time);

	tm->tm_hour = 4;
	tm->tm_min = 0;
	tm->tm_sec = 0;

	if(CHECK_LEAP_YEAR(tm->tm_year))
	{
		days[1] = 29;
	}
	else
	{
		days[1] = 28;
	}

	switch (ResetType)
	{
	case ResetType_Daily:
	{
		tm->tm_mday++;
	}	break;

	case ResetType_Weekly:
	{
		tm->tm_mday += 7;
		int wday = 1 - tm->tm_wday;
		tm->tm_mday += wday;
	}	break;

	case ResetType_Monthly:
	{
		tm->tm_mday = 1;
		tm->tm_mon++;

		if(tm->tm_mon == 12)
		{
			tm->tm_mon = 0;
			tm->tm_year++;
		}

		return mktime(tm);
	}	break;
	
	default:
		break;
	}
	
	if(tm->tm_mday > days[tm->tm_mon])
	{
		int overDay = days[tm->tm_mon] - tm->tm_mday;
		tm->tm_mday = overDay * -1;
		tm->tm_mon++;

		if(tm->tm_mon == 12)
		{
			tm->tm_mon = 0;
			tm->tm_year++;
		}
	}


	return mktime(tm);
}

EventStruct* EventTimerWheel::operator[] (uint64 eventKey)
{
	int res;
	eventKey = GET_HASH_KEY(eventKey);
	res = GET_HASH_MEM(m_shmTimerEvent, eventKey, selectedEvent);
	if(res < 0)
	{
		res = NEW_HASH_MEM(m_shmTimerEvent, eventKey, selectedEvent);
		if(res < 0)
		{
			dAppLog(LOG_DEBUG, "New Event Hash shm Error [res : %d][eventKey : %llx]", res, eventKey);
			return NULL;
		}
	}

	return selectedEvent;
}