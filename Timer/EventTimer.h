#ifndef __EVENT_TIMER_WHEEL__
#define __EVENT_TIMER_WHEEL__

#include <ctime>
#include <sys/timeb.h>
#include <Thread.h>

#include <lwrite.h>
#include <hashshm.h>
#include <shmutil.h>
#include <SharedMemory.h>

#include "World.h"
#include "GameCoreDefine.h"

#include "EventTimerFunction.h"

#define uint64 unsigned long long

#define TIMER_WHEEL_TICK_PER_SECOND 10
#define TIMER_WHEEL_TICK_TIME (1000/TIMER_WHEEL_TICK_PER_SECOND) // (millisecond) 1000의 약수가 아니면 오차 발생
#define TIMER_WHEEL_SIZE 8
#define TIMER_WHEEL_EVENT_SIZE 60000
#define TIMER_WHEEL_EVENT_SIZE_HOUR 60000
#define TIMER_WHEEL_EVENT_SIZE_MINUTE 10000
#define TIMER_WHEEL_EVENT_SIZE_SECOND 10000

#define TIME_DAY				86400
#define TIME_HOUR				3600
#define TIME_MINUTE				60
#define MILLISECOND_PER_DAY		86400000
#define MILLISECOND_PER_WEEK	604800000

#define	GET_TIMER_KEY(slot)	(((uint64)slot << 48)+1)

#define CHECK_LEAP_YEAR(year) (((year+1900)%4==0 && (year+1900)%100!=0) || (year+1900)%400==0)

#define MAKE_MINUTE_TO_SECOND(minute) (TIME_MINUTE * minute)

#define TID_RESET_DAILY		20
#define TID_RESET_WEEK		21
#define TID_RESET_MONTH		23

typedef int (*TimerFunc)();

struct EventStruct
{
	uint64		preEventKey;
	uint64		nextEventKey;
	uint64		tid;
	uint64		millisecond;
	int			leftCount;
	int			leftMilliSecond;
	uint64		functionId;
	uint64		eventVar;
	uint64		triggerTime;
	bool		repeat;
	int			repeatCount;
};

struct TimerRemoteControler
{
	int (*addTimeOut) (uint64 func, uint64 millisecond, uint64 eventVar, bool repeat, int repeatCount, uint64 tid, bool overriding);
	int (*addTimeOutDelay) (uint64 func, uint64 startTime, uint64 eventVar, bool repeat, int repeatCount, uint64 repeatDelay, uint64 tid, bool overriding);

	int (*tick) (void);
	int (*stop) (void);
	int (*removeUVS) (uint64 tid);
	int (*remove) (uint64 tid);

	int (*getEvent) (uint64 tid, EventStruct **event);

	UINT64 (*getTriggerTime) (uint64 tid);
	UINT64 (*getLeftTime) (uint64 tid);
	UINT64 (*getCurrentTimeFormat) (void);
	tm* (*getCurrentTimeTM) (void);
	UINT64 (*getCurrentSecondFromTime) (void);
	UINT64 (*getMillisecondFromCurrentTime) (tm *time);
	UINT64 (*getMillisecondFromCurrentTime2) (time_t *time);
	UINT64 (*getDayAfterValue) (unsigned char value);
	time_t (*getDayAfterResetType) (unsigned char ResetType);
	int (*getLastDayofMonth) (int Year, int Month);
};

static bool m_start = false;

static int days[12] = {31, 28, 31, 30, 31, 30, 31, 31 ,30 ,31 ,30 ,31};

// 50ms 단위로 돌아가는 타이머 휠. keep alive 패킷, 핑퐁, 연결 지연등 타임아웃 이벤트.
// 또는 서버에서 반복적으로 호출 되어야 하는 이벤트들을 관리 할 것임.
class EventTimerWheel
{
public :
	EventTimerWheel();
	~EventTimerWheel();

	int InitSharedMemory();

	static int AddTimeOut(uint64 functionId, uint64 millisecond, uint64 eventVar, bool repeat = false, int repeatCount = -1, uint64 tid = 0, bool overriding = false);
	static int AddTimeOut(uint64 functionId, uint64 startTime, uint64 eventVar, bool repeat, int repeatCount, uint64 repeatDelay, uint64 tid, bool overriding = false);

	static int RemoveUVSEvent(uint64 tid);
	static int RemoveEvent(uint64 tid);

	static int RunEvent(EventStruct *st_evnet);

	static int GetEvent(uint64 eventKey, EventStruct **event);

	static UINT64 GetTriggerTime(uint64 tid);
	static UINT64 GetLeftTime(uint64 tid);
	static UINT64 GetCurrentTimeUINT64();
	static tm* GetCurrentTimeTM();
	static UINT64 GetCurrentSecondFromTime();
	static UINT64 GetDayAfterValue(UCHAR value);
	static time_t GetDayAfterResetType(UCHAR ResetType);
	static int GetLastDayofMonth(int Year, int Month);


	static UINT64 GetMillisecondFromCurrentTime(tm *time);
	static UINT64 GetMillisecondFromCurrentTime(time_t *time);
	static UINT64 GetMillisecondFromUINT64(UINT64 time);

private :

	static bool flag;

	static USHORT eventSerial;

	static st_HashedShmHandle m_shmTimerEvent;
	st_HashedShmHandle m_smTimerWheelHour;
	st_HashedShmHandle m_smTimerWheelMinute;
	st_HashedShmHandle m_smTimerWheelSecond;

	// // 카운트
	// int hourValueNum[TIMER_WHEEL_SIZE];
	// int minuteValueNum[TIMER_WHEEL_SIZE];
	// int secondValueNum[TIMER_WHEEL_SIZE];

	static UINT32 *hourHead[TIMER_WHEEL_SIZE], *minuteHead[TIMER_WHEEL_SIZE], *secondHead[TIMER_WHEEL_SIZE];
	static UINT32 *hourPointer, *minutePointer, *secondPointer;
	static UINT32 minuteCurrent, secondCurrent;

	static EventStruct *selectedEvent;

	static int Tick();

	EventStruct* operator[] (uint64 eventKey);
};

extern struct TimerRemoteControler *timer;

#endif
