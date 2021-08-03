#ifndef __EVENT_TIMER_WHEEL__
#define __EVENT_TIMER_WHEEL__

#include <ctime>
#include <sys/timeb.h>
#include <thread>

#include <lwrite.h>
#include <hashshm.h>
#include <shmutil.h>
#include <SharedMemory.h>

#include "World.h"
#include "GameCoreDefine.h"

#include "EventTimerFunction.h"

#define uint64 unsigned long long

#define TIMER_WHEEL_TICK_PER_SECOND 100
#define TIMER_WHEEL_TICK_TIME (CLOCKS_PER_SEC/TIMER_WHEEL_TICK_PER_SECOND) // (millisecond) 1000의 약수가 아니면 오차 발생
#define TIMER_WHEEL_SIZE 8
#define TIMER_WHEEL_EVENT_SIZE 60000
#define TIMER_WHEEL_EVENT_SIZE_HOUR 60000
#define TIMER_WHEEL_EVENT_SIZE_MINUTE 10000
#define TIMER_WHEEL_EVENT_SIZE_SECOND 10000

#define	GET_TIMER_KEY(slot)	(((uint64)slot << 48)+1)

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
	bool		repeat;
	int			repeatCount;
};

struct TimerRemoteControler
{
	int (*addTimeOut) (uint64 func, uint64 millisecond, uint64 eventVar, bool repeat, int repeatCount, uint64 tid, bool overriding);

	int (*tick) (void);
	int (*stop) (void);
	int (*remove) (uint64 tid);
};

static bool m_start = false;

// 50ms 단위로 돌아가는 타이머 휠. keep alive 패킷, 핑퐁, 연결 지연등 타임아웃 이벤트.
// 또는 서버에서 반복적으로 호출 되어야 하는 이벤트들을 관리 할 것임.
class EventTimerWheel
{
public :
	EventTimerWheel();
	~EventTimerWheel();

	int InitSharedMemory();

	static int AddTimeOut(uint64 functionId, uint64 millisecond, uint64 eventVar, bool repeat = false, int repeatCount = -1, uint64 tid = 0, bool overriding = false);

	static int RemoveEvent(uint64 tid);

	static int RunEvent(uint64 functionId, uint64 eventVar);

	static int GetEvent(uint64 eventKey, EventStruct **event);

private :

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
