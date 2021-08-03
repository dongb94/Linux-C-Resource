
#include <EventTimerFunction.h>

int AddHp(UINT64 CharSmKey)
{
	userCoreSTI->PlayerStateChange(CharSmKey, EventType_HpRecovery, 100);
	return 0;
}

int TestEvent()
{
	userCoreSTI->EventTest(3);
	return 0;
}

int TestEvent2()
{
	userCoreSTI->EventTest(10);
	return 0;
}

int TestEvent3()
{
	userCoreSTI->EventTest(100);
	return 0;
}