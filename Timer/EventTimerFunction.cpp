
#include <EventTimerFunction.h>

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

int DisconnectLogout(UINT64 UserSmKey) 
{
	// printf("LOGOUT USER [%llx]\n", UserSmKey);
	userCoreSTI->LogoutUser(UserSmKey, 0, 0, ERROR_NO_REPLY);
	return 0;
}

int AddHp(UINT64 CharSmKey)
{
	userCoreSTI->PlayerStateChange(CharSmKey, EventType_HpRecovery, 100);
	return 0;
}

int ExpDoubleBuffEnd(UINT64 CharSmKey)
{
	printf("BUFF END!\n");
	int res;
	X_USER_CHARACTER_INFO *pCharInfo;
	res = Get_hashed_shm(&smhUserCharacterInfo, CharSmKey, (void**)&pCharInfo);
	if(res >= 0)
	{
		for(int i = 0; i<MAX_BUFF_COUNT; i++)
		{
			if(pCharInfo->buffInfo[i].uiBuffId == EventType_ExpDouble)
			{
				memset(&(pCharInfo->buffInfo[i]), 0, sizeof(X_BUFF_INFO));
				pCharInfo->additionalStatusInfo[EXP_Multiplier] = 0;
			}
		}
	}
	userCoreSTI->PlayerStateChange(CharSmKey, EventType_ExpDouble, 0);
}

int UpdateSellItemDB(USHORT usShopResetType)
{
	dAppLog(LOG_DEBUG, "UPDATE SELL ITEM DB [ShopResetType : %d]", usShopResetType);
	userCoreSTI->ResetShopPurchaseInfo(usShopResetType);
}