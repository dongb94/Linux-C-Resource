
#ifndef EVENT_TIMER_FFUNCTIONS__
#define EVENT_TIMER_FFUNCTIONS__

#include <typedef.h>
#include <GameCoreDefine.h>

#include "UserCore.h"

#define TIMER_EVENT_TEST			999
#define TIMER_EVENT_TEST2			1000
#define TIMER_EVENT_TEST3			1001

#define TIMER_FUNCTION_DISCONNECT_USER_LOGOUT	42
#define TIMER_FUNCTION_ID_ADD_HP				100
#define TIMER_FUNCTION_EXP_DOUBLE_BUFF_END		1800 // 경험치 2배 포션

int TestEvent();
int TestEvent2();
int TestEvent3();

int DisconnectLogout(UINT64 UserSmKey);
int AddHp(UINT64 CharSmKey);
int ExpDoubleBuffEnd(UINT64 CharSmKey);


#endif // EVENT_TIMER_FFUNCTIONS__