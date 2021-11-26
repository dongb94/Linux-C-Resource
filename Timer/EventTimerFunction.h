
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

#define TIMER_FUNCTION_RESET_SHOP_DAILY_PURCHASE		2000
#define TIMER_FUNCTION_RESET_SHOP_WEEKLY_PURCHASE		2001
#define TIMER_FUNCTION_RESET_SHOP_MONTHLY_PURCHASE		2002
#define TIMER_FINCTION_RESET_SHOP_FIXEDTIME_PURCHASE	2003

int TestEvent();
int TestEvent2();
int TestEvent3();

int DisconnectLogout(UINT64 UserSmKey);
int AddHp(UINT64 CharSmKey);
int ExpDoubleBuffEnd(UINT64 CharSmKey);

int UpdateSellItemDB(USHORT usShopResetType);


#endif // EVENT_TIMER_FFUNCTIONS__