#include "EventTimer.h"


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
	timer->tick = Tick;
	timer->remove = RemoveEvent;
	timer->getCurrentTime = GetCurrentTime;

	eventSerial = 1;

	InitSharedMemory();

	if(!m_start)
	{
		m_start = true;

		std::thread t([=]() 
		{
			// 나중에 clock 을 때고 ftime 기반으로 대체해야함. clock은 오차가 있음.
			clock_t tick;
			tick = clock();

			struct timeb m_time;
			ftime(&m_time);
			struct tm *tm_ptr;
			tm_ptr = localtime(&m_time.time);
			printf("\tTimer Start Time : %d:%d:%d\n", tm_ptr->tm_hour, tm_ptr->tm_min, tm_ptr->tm_sec);

			minuteCurrent = (UINT32)tm_ptr->tm_min;
			secondCurrent = (UINT32)(tm_ptr->tm_sec * (TIMER_WHEEL_TICK_PER_SECOND));
			secondCurrent += m_time.millitm * TIMER_WHEEL_TICK_PER_SECOND / 1000;

			long clockTime;
			while(true)
			{
				// ftime(&m_time);
				clockTime = clock();
				// dAppLog(LOG_DEBUG, "[clock %d]-[tick %d]=[%d]", clockTime, tick, clockTime - tick);
				// dAppLog(LOG_DEBUG, "Delay %d > %d [TIME : %d]", clockTime - tick, CLOCKS_PER_SEC, clockTime/CLOCKS_PER_SEC);
				if(clockTime-tick > TIMER_WHEEL_TICK_TIME)
				{
					// dAppLog(LOG_DEBUG, "[clock %d]-[tick %d]=[%d]", clockTime, tick, clockTime - tick);
					tick += TIMER_WHEEL_TICK_TIME;
					if(timer->tick()<0)
					{
						m_start = false;
						return;
					}
				}

				usleep(1000);
			}
		});
		t.detach();
	}
}

EventTimerWheel::~EventTimerWheel(){}

int EventTimerWheel::InitSharedMemory()
{
	int res;
	UINT64 *headKey;
	UINT64 key;

	dAppLog(LOG_DEBUG, "Init Event Timer SharedMemory");

	Init_hashed_shm_handle(&m_shmTimerEvent, TIMER_WHEEL_EVENT_SIZE, sizeof(EventStruct), TIMER_WHEEL_EVENT_SIZE, S_SSHM_EVENT_TIMEWHEEL);
	res = Init_hashed_shm(&m_shmTimerEvent);
	if(res < 0)
	{
		dAppLog(LOG_DEBUG, "Init Event Timer SharedMemory Error [res : %d]\n", res);
		return res;
	}

	////////////////////////////////////////////
	////////////////////////////////////////////

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

int EventTimerWheel::AddTimeOut(uint64 functionId, uint64 millisecond, uint64 eventVar, bool repeat, int repeatCount, uint64 tid, bool overrriding)
{
	if(millisecond == 0)
	{
		RunEvent(functionId, eventVar);
		return 1;
	}

	int res;
	EventStruct* newEvent;

	if(tid == 0)
	{
		tid = eventSerial + 10000;
		eventSerial = (eventSerial+1) % (TIMER_WHEEL_EVENT_SIZE-10000);
	}

	res = GetEvent(tid, &newEvent);
	if(res < 0)
	{
		dAppLog(LOG_CRI, "Get Timer Event Error [tid : %lld]", tid);
		return -1;
	}

	while(newEvent->tid != 0)
	{
		if(overrriding) break;
		tid = (tid + 1) % TIMER_WHEEL_EVENT_SIZE;
		if(tid == 0) tid = 10000;

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
	newEvent->millisecond = millisecond;
	newEvent->functionId = functionId;
	newEvent->eventVar = eventVar;
	newEvent->repeat = repeat;
	newEvent->repeatCount = repeatCount;

	int hour = millisecond / 3600000;
	int minute = millisecond / 60000;
	UINT32 *HeadKeyPointer = NULL;

	if(hour != 0){
		int hourSlot = ((hour+TIMER_WHEEL_SIZE-1) + *hourPointer) & 0b111;		// == hour % 8

		HeadKeyPointer = hourHead[hourSlot];
		if(HeadKeyPointer == NULL)
		{
			dAppLog(LOG_CRI, "Get Hour Event Shared Memory Error [hourslot : %d][key : %llx]", hourSlot, GET_TIMER_KEY(hourSlot));
			return -3;
		}

		newEvent->leftCount = hour >> 3;	 // == hour / 8
		newEvent->leftMilliSecond = (millisecond % 3600000) + (minuteCurrent * 60000) + (secondCurrent * 1000 / TIMER_WHEEL_TICK_PER_SECOND);
	}
	else if(minute != 0){
		int minuteSlot = ((minute+TIMER_WHEEL_SIZE-1) + *minutePointer) & 0b111;

		HeadKeyPointer = minuteHead[minuteSlot];
		if(HeadKeyPointer == NULL)
		{
			dAppLog(LOG_CRI, "Get Minute Event Shared Memory Error [minuteSlot : %d][key : %llx]", minuteSlot, GET_TIMER_KEY(minuteSlot));
			return -3;
		}

		newEvent->leftCount = minute >> 3;
		newEvent->leftMilliSecond = (millisecond % 60000) + (secondCurrent * 1000 / TIMER_WHEEL_TICK_PER_SECOND);
	}
	else{
		int timeTick = millisecond * TIMER_WHEEL_TICK_PER_SECOND / 1000;
		int secondSlot = (timeTick + *secondPointer) & 0b111;

		HeadKeyPointer = secondHead[secondSlot];
		if(HeadKeyPointer == NULL)
		{
			dAppLog(LOG_CRI, "Get Second Event Shared Memory Error [secondSlot : %d][key : %llx]", secondSlot, GET_TIMER_KEY(secondSlot));
			return -3;
		}

		newEvent->leftCount = timeTick >> 3;
		newEvent->leftMilliSecond = millisecond % (1000 / TIMER_WHEEL_TICK_PER_SECOND);

		dAppLog(LOG_DEBUG, " Add Time Tick [millisecond %lld][tiemTick %d][secondSlot %d][leftCount %d][secondPointer %d]", millisecond, timeTick, secondSlot, newEvent->leftCount, *secondPointer);
	}

	if(HeadKeyPointer == NULL)
	{
		dAppLog(LOG_CRI, "[[HeadKeyPointer was NULL]]");
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
			return -1;
		}

		newEvent->nextEventKey = *HeadKeyPointer;
		selectedEvent->preEventKey = tid;
		*HeadKeyPointer = tid;
	}

	// printf("Add New Event [tid : %d]\n", newEvent->tid);
	dAppLog(LOG_DEBUG, "Add New Event [pre %lld][next %lld][tid %lld][millisecond %lld][function %lld][var1 %lld][repeat %d][repeat count %d][%d:%d]", 
						newEvent->preEventKey, newEvent->nextEventKey, newEvent->tid, newEvent->millisecond, newEvent->functionId, eventVar, newEvent->repeat, newEvent->repeatCount,
						hour, minute);

	return newEvent->tid;
}

int EventTimerWheel::RemoveEvent(uint64 tid)
{
	if(tid < 1 || tid >= TIMER_WHEEL_EVENT_SIZE) return 0;

	EventStruct *st_event, *st_preEvent, *st_nextEvent;
	GetEvent(tid, &st_event);

	if(st_event == NULL)
	{
		printf("NO Remove Event [tid %d]", tid);
		return -1;
	}

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

	return 0;
}

int EventTimerWheel::Tick()
{
	int res;
	INT64 key;
	UINT32 HeadKeyPointer;

	EventStruct *st_eventGroup;
	EventStruct *st_event;
	EventStruct	*st_preEvent, *st_nextEvent;

	if(secondPointer == NULL) return 0;
	// printf("tick [%d:%02d:%04d]\n", *hourPointer, minuteCurrent, secondCurrent);
	HeadKeyPointer = *(secondHead[*secondPointer]);
	// printf("%d\n", HeadKeyPointer);
	while(HeadKeyPointer != 0)
	{
		GetEvent(HeadKeyPointer, &st_event);
		if(st_event->tid == 0)
		{
			dAppLog(LOG_CRI, "event error [%d]", HeadKeyPointer);
			return -1;
		}
		else if(st_event->leftCount <=0)
		{
			// printf("Run [%d][Func %d]\n", st_event->tid, st_event->functionId);
			RunEvent(st_event->functionId, st_event->eventVar);
			
			// 원소 제거
			if(*(secondHead[*secondPointer]) == st_event->tid)
			{
				*(secondHead[*secondPointer]) = st_event->nextEventKey;
			}
			HeadKeyPointer = st_event->nextEventKey;

			if(st_event->preEventKey != 0)
			{
				GetEvent(st_event->preEventKey, &st_preEvent);
				st_preEvent->nextEventKey = st_event->nextEventKey;
			}
			if(st_event->nextEventKey != 0)
			{
				GetEvent(st_event->nextEventKey, &st_nextEvent);
				st_nextEvent->preEventKey = st_event->preEventKey;
			}

			if(st_event->repeat)
			{
				if(st_event->repeatCount < 0)	// infinity repeat
					AddTimeOut(st_event->functionId, st_event->millisecond, st_event->eventVar, true, -1, st_event->tid, true);
				else if(st_event->repeatCount > 0)
					AddTimeOut(st_event->functionId, st_event->millisecond, st_event->eventVar, true, st_event->repeatCount-1, st_event->tid, true);
			}
			else{
				memset(st_event, 0, sizeof(EventStruct));
			}

			continue;
		}
		else
		{
			// printf("[TID %lld][Tick %d][left %d]\n", st_event->tid, *secondPointer, st_event->leftCount);
			st_event->leftCount--;
		}

		HeadKeyPointer = st_event->nextEventKey;
		//printf("%d\n", HeadKeyPointer);
	}

	*secondPointer = (*secondPointer + 1) & 0b111;
	secondCurrent = (secondCurrent + 1) % (60 * TIMER_WHEEL_TICK_PER_SECOND);
	if(secondCurrent != 0) return 0;

	HeadKeyPointer = *(minuteHead[*minutePointer]);
	while(HeadKeyPointer != 0)
	{
		GetEvent(HeadKeyPointer, &st_event);
		if(st_event->leftCount <=0)
		{
			// 원소 제거
			if(st_event->preEventKey != 0)
			{
				GetEvent(st_event->preEventKey, &st_preEvent);
				st_preEvent->nextEventKey = st_event->nextEventKey;
			}
			if(st_event->nextEventKey != 0)
			{
				GetEvent(st_event->nextEventKey, &st_nextEvent);
				st_nextEvent->preEventKey = st_event->preEventKey;
			}

			// 타이머 휠 갱신
			AddTimeOut(st_event->functionId, st_event->leftMilliSecond, st_event->eventVar, st_event->repeat, st_event->repeatCount, st_event->tid, true);

			if(*(minuteHead[*minutePointer]) == st_event->tid)
			{
				*(minuteHead[*minutePointer]) = st_event->nextEventKey;
				continue;
			}
		}
		else
		{
			st_event->leftCount--;
		}

		HeadKeyPointer = st_event->nextEventKey;
	}

	*minutePointer = (*minutePointer + 1) & 0b111;
	minuteCurrent = (minuteCurrent + 1) % 60;
	if(minuteCurrent != 0) return 1;

	HeadKeyPointer = *(hourHead[*hourPointer]);
	while(HeadKeyPointer != 0)
	{
		GetEvent(HeadKeyPointer, &st_event);
		if(st_event->leftCount <=0)
		{
			// 원소 제거
			if(st_event->preEventKey != 0)
			{
				GetEvent(st_event->preEventKey, &st_preEvent);
				st_preEvent->nextEventKey = st_event->nextEventKey;
			}
			if(st_event->nextEventKey != 0)
			{
				GetEvent(st_event->nextEventKey, &st_nextEvent);
				st_nextEvent->preEventKey = st_event->preEventKey;
			}

			// 타이머 휠 갱신
			AddTimeOut(st_event->functionId, st_event->leftMilliSecond, st_event->eventVar, st_event->repeat, st_event->repeatCount, st_event->tid, true);

			if(*(hourHead[*hourPointer]) == st_event->tid)
			{
				*(hourHead[*hourPointer]) = st_event->nextEventKey;
				continue;
			}
		}
		else
		{
			st_event->leftCount--;
		}

		HeadKeyPointer = st_event->nextEventKey;
	}

	*hourPointer = (*hourPointer + 1) & 0b111;

	return 2;
}

int EventTimerWheel::RunEvent(uint64 functionId, uint64 eventVar)
{
	switch (functionId)
	{
	case TIMER_FUNCTION_ID_ADD_HP:
		AddHp(eventVar);
		break;

	case TIMER_EVENT_TEST:
		TestEvent();
		break;
	
	case TIMER_EVENT_TEST2:
		TestEvent2();
		break;

	case TIMER_EVENT_TEST3:
		TestEvent3();
		break;

	case TIMER_FUNCTION_EXP_DOUBLE_BUFF_END:
		ExpDoubleBuffEnd(eventVar);
		break;

	default:
		break;
	}

	return 0;
}

int EventTimerWheel::GetEvent(uint64 eventKey, EventStruct **event)
{
	int res;
	eventKey = GET_HASH_KEY(eventKey);
	res = Get_hashed_shm(&m_shmTimerEvent, eventKey, (void**)event);
	if(res < 0)
	{
		res = New_hashed_shm(&m_shmTimerEvent, eventKey, (void**)event);
		if(res < 0)
		{
			dAppLog(LOG_DEBUG, "New Event Hash shm Error [res : %d][eventKey : %llx]", res, eventKey);
			return -1;
		}
	}

	return 0;
}

int EventTimerWheel::GetCurrentTime()
{
	return clock()*1000/CLOCKS_PER_SEC;
}

EventStruct* EventTimerWheel::operator[] (uint64 eventKey)
{
	int res;
	eventKey = GET_HASH_KEY(eventKey);
	res = Get_hashed_shm(&m_shmTimerEvent, eventKey, (void**)selectedEvent);
	if(res < 0)
	{
		res = New_hashed_shm(&m_shmTimerEvent, eventKey, (void**)selectedEvent);
		if(res < 0)
		{
			dAppLog(LOG_DEBUG, "New Event Hash shm Error [res : %d][eventKey : %llx]", res, eventKey);
			return NULL;
		}
	}

	return selectedEvent;
}