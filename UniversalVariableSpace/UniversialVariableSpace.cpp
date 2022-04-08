#include "UniversialVariableSpace.h"


CUniversalVariableSpace::CUniversalVariableSpace()
{
	m_InfoHead = m_InfoTail = 0;
	m_CurrentInfoIndex	= -1;
	m_InfoCount		= 0;
}

CUniversalVariableSpace::~CUniversalVariableSpace(){}

void CUniversalVariableSpace::GetData(int iUVIIndex)
{
	pst_Universal_Variable_Info pstUVI = P_INFO(iUVIIndex);

	memcpy(m_stBuffer.szBody, P_DATA(pstUVI->DataIndex), pstUVI->ucDataSize);
	m_stBuffer.ucDataSize = 0;

	return;
}

/// return dataInfo index
int CUniversalVariableSpace::AddData(unsigned char size, char* data)
{
	if(size == 0) return INPUT_DATA_ERROR_SIZE_ZERO;

	pst_Universal_Variable_Info pstUVIIter;

	int useableSize = 0;
	int dataStartIndex = 0;
	bool isIndexFull = false;
	bool isDataFull = false;

	if(m_InfoCount == 0) // 데이터가 비어있을때
	{
		if(size > MAX_UNIVERSAL_VARIABLE_SPACE) return UNIVERSIAL_VARIABLE_ERROR_SPACE_FULL;
		
		m_CurrentInfoIndex = 0;
		pstUVIIter = CURRENT_INFO;

		m_InfoHead = m_InfoTail = 0;
		pstUVIIter->iPreInfo = pstUVIIter->iNextInfo = -1;
		pstUVIIter->DataIndex = 0;
		pstUVIIter->ucDataSize = size;
		memcpy(P_DATA(dataStartIndex), data, size);
		m_InfoCount++;

		return m_CurrentInfoIndex;
	}

	if(m_CurrentInfoIndex == -1)
	{
		dAppLog(LOG_CRI, "**** UNIVERSAIL VARIABLE CRITICAL ERROR : COUNT NOT ZERO BUT CURRENT_INFO_INDEX -1");
		return CURRUNT_INDEX_IS_NULL;
	}
	
	//
	if(m_CurrentInfoIndex == MAX_UNIVERSAL_VARIABLE_INFO)
	{
		if(isIndexFull) return UNIVERSIAL_VARIABLE_ERROR_INDEX_FULL;
		isIndexFull = true;
		isDataFull = true;
		pstUVIIter = HEAD_INFO;
		dataStartIndex = 0;
		useableSize = 0;
		m_CurrentInfoIndex = 0;
	}
	//

	// 빈 데이터 공간 찾기
	pstUVIIter = CURRENT_INFO;
	dataStartIndex = DATA_END_INDEX(pstUVIIter);

	while(useableSize < size)	
	{
		if(pstUVIIter->iNextInfo == -1 || dataStartIndex + size >= MAX_UNIVERSAL_VARIABLE_SPACE)
		{
			//
			if(pstUVIIter->ucDataSize)
			{
				dataStartIndex = DATA_END_INDEX(pstUVIIter);
			}
			//
			useableSize = MAX_UNIVERSAL_VARIABLE_SPACE - dataStartIndex;
			if(useableSize < size)
			{
				if(isDataFull) return UNIVERSIAL_VARIABLE_ERROR_SPACE_FULL;
				isDataFull = true;
				pstUVIIter = HEAD_INFO;
				dataStartIndex = 0;
				useableSize = 0;
				continue;
			}
			break;
		}

		//
		if(pstUVIIter->ucDataSize)
		{
			dataStartIndex = DATA_END_INDEX(pstUVIIter);
			if(pstUVIIter->iPreInfo == -1 && pstUVIIter->DataIndex != 0)
			{
				dataStartIndex = MemoryFree(0, pstUVIIter);
			}

			useableSize = NEXT_INFO(pstUVIIter)->DataIndex - DATA_END_INDEX(pstUVIIter);
			if(useableSize >= size)
			{
				break;
			}

			if(DATA_END_INDEX(pstUVIIter) != NEXT_INFO(pstUVIIter)->DataIndex)
			{
				dataStartIndex = MemoryFree(DATA_END_INDEX(pstUVIIter), NEXT_INFO(pstUVIIter));
			}

			pstUVIIter = NEXT_INFO(pstUVIIter);
			continue;
		}
		//

		pstUVIIter = NEXT_INFO(pstUVIIter);
		useableSize = DATA_FRONT_INDEX(pstUVIIter) - dataStartIndex;
		if(DATA_FRONT_INDEX(pstUVIIter) != dataStartIndex)
		{
			dataStartIndex = MemoryFree(dataStartIndex, pstUVIIter);
		}
	}
	
	// 빈 인포 리스트 찾기
	pstUVIIter = CURRENT_INFO;
	while(pstUVIIter->ucDataSize)
	{
		m_CurrentInfoIndex++;
		if(m_CurrentInfoIndex == MAX_UNIVERSAL_VARIABLE_INFO)
		{
			if(isIndexFull) return UNIVERSIAL_VARIABLE_ERROR_INDEX_FULL;
			isIndexFull = true;
			m_CurrentInfoIndex = 0;
		}
		pstUVIIter = CURRENT_INFO;
	}

	pstUVIIter->DataIndex = dataStartIndex;
	pstUVIIter->ucDataSize = size;
	//
	TAIL_INFO->iNextInfo = m_CurrentInfoIndex;
	//
	pstUVIIter->iPreInfo = m_InfoTail;
	pstUVIIter->iNextInfo = -1;

	memcpy(P_DATA(dataStartIndex), data, size);
	m_InfoTail = m_CurrentInfoIndex;
	m_InfoCount++;

	SortList(m_CurrentInfoIndex);

	return m_CurrentInfoIndex;
}

int CUniversalVariableSpace::RemoveData(int iUVIIndex)
{
	return RemoveIndex(P_INFO(iUVIIndex));
}

int CUniversalVariableSpace::RemoveIndex(pst_Universal_Variable_Info pstUVI)
{
	if(pstUVI->ucDataSize == 0) return 0;

	if(pstUVI == CURRENT_INFO)
		m_CurrentInfoIndex = (pstUVI->iPreInfo == -1)?pstUVI->iNextInfo:pstUVI->iPreInfo;

	if(pstUVI->iPreInfo != -1)
		PRE_INFO(pstUVI)->iNextInfo = pstUVI->iNextInfo;
	else
		m_InfoHead = pstUVI->iNextInfo;

	if(pstUVI->iNextInfo != -1)
		NEXT_INFO(pstUVI)->iPreInfo = pstUVI->iPreInfo;
	else
		m_InfoTail = pstUVI->iPreInfo;
	memset(pstUVI, 0, sizeof(st_Universal_Variable_Info));
	m_InfoCount--;

	return 0;
}

/// 메모리 조각 모음 (빈 공간 제거)
/// return 옮겨진 데이터의 마지막 인덱스
int CUniversalVariableSpace::MemoryFree(int moveIndex, pst_Universal_Variable_Info pMoveUVI)
{
	memmove(P_DATA(moveIndex), P_DATA(DATA_FRONT_INDEX(pMoveUVI)), pMoveUVI->ucDataSize);
	pMoveUVI->DataIndex = moveIndex;

	return DATA_END_INDEX(pMoveUVI);
}

int CUniversalVariableSpace::SortList(int iUVIIndex)
{
	pst_Universal_Variable_Info iter = P_INFO(iUVIIndex);
	pst_Universal_Variable_Info pre;

	if(iter->iPreInfo == -1) return 0;	// iUVIIndex == m_InfoHead

	while(PRE_INFO(iter)->DataIndex > iter->DataIndex)
	{
		if(iter->iNextInfo == -1)
		{
			m_InfoTail = iter->iPreInfo;
		}

		pre = PRE_INFO(iter);

		pre->iNextInfo = iter->iNextInfo;
		if(iter->iNextInfo != -1)
			NEXT_INFO(iter)->iPreInfo = iter->iPreInfo;
		iter->iNextInfo = iter->iPreInfo;
		iter->iPreInfo = pre->iPreInfo;
		pre->iPreInfo = iUVIIndex;
		if(iter->iPreInfo != -1)
			PRE_INFO(iter)->iNextInfo = iUVIIndex;

		if(iter->iPreInfo == -1) 
		{
			m_InfoHead = iUVIIndex;
			break;
		}
	}

	return 0;
}