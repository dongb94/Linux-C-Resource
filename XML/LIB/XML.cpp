#include <XML.h>

#ifndef MAKE_SHM_KEY_FROM_INDEX
#define MAKE_SHM_KEY_FROM_INDEX(index) (((unsigned long long)index)<<48)+1
#endif

int XML::SetXMLTarget(unsigned int uiEntitySize, unsigned int uiEntityNum, unsigned long long shmKey)
{
	Init_hashed_shm_handle(&m_pHandle, uiEntityNum, uiEntitySize, uiEntityNum, shmKey);
	int res = Init_hashed_shm(&m_pHandle);
	if(res < 0)
	{
		dAppLog(LOG_CRI, "[XML] Init SharedMemory Error [shmkey : %lld][res : %d]", shmKey, res);
	}

	maxSize = uiEntityNum;

	return 0;
}

int XML::At(unsigned long long index, XML_BASE** ppData)
{
	int res = Get_hashed_shm(&m_pHandle, MAKE_SHM_KEY_FROM_INDEX(index), (void**)ppData);
	if(res < 0)
	{
		dAppLog(LOG_DEBUG, "get XML Hash shm Error [shmkey : %lld][res : %d][index : %d]", m_pHandle.kShmKey, res, index);
		return res;
	}
	return 0;
}

// XML KEY 값으로 이분 검색
/// Return - 성공 1, 실패 음수값
int XML::GetXMLData(unsigned long long dataKey, XML_BASE** ppData)
{
	int low, high, searchPoint;
	unsigned long long foundKey;
	low = 0;
	high = maxSize;
	while(true)
	{
		searchPoint = (low + high) / 2;

		int res = Get_hashed_shm(&m_pHandle, MAKE_SHM_KEY_FROM_INDEX(searchPoint), (void**)ppData);
		if(res < 0)
		{
			if(res == HASHED_SHM_ERR_NOT_FOUND)
			{
				foundKey = dataKey + 1;
			}
			else
			{
				dAppLog(LOG_DEBUG, "Get XML Hash shm Error [res : %d][SP : %d][TargetKEY : %d]", res, searchPoint, dataKey);
				return -1;
			}
		}
		else
		{
			foundKey = (*ppData)->Key;
		}

		if(foundKey == dataKey)
		{
			return 1;
		}
		else if(foundKey > dataKey)
		{
			high = searchPoint - 1;
		}
		else
		{
			low = searchPoint + 1;
		}

		if(high < low) {
			// dAppLog(LOG_DEBUG, "XML Data Not Set [%d:%d][target : %d][current : %d]", low, high, dataKey, (*ppData)->Key);
			return -3;
		}
	}

	return -2;
}