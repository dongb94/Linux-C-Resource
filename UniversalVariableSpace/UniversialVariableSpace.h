#ifndef __UNIVERSAL_VARIABLE_SPACE__
#define __UNIVERSAL_VARIABLE_SPACE__

#define MAX_UNIVERSAL_VARIABLE_MAX_BODY_LEN	100
#define MAX_UNIVERSAL_VARIABLE_INFO			20000
#define MAX_UNIVERSAL_VARIABLE_SPACE		320000

#define INPUT_DATA_ERROR_SIZE_ZERO				-1
#define UNIVERSIAL_VARIABLE_ERROR_INDEX_FULL	-2
#define UNIVERSIAL_VARIABLE_ERROR_SPACE_FULL	-3
#define CURRUNT_INDEX_IS_NULL					-4

#define P_INFO(INDEX)				(&(stUVIArray[INDEX]))
#define HEAD_INFO					(&(stUVIArray[m_InfoHead]))
#define TAIL_INFO					(&(stUVIArray[m_InfoTail]))
#define PRE_INFO(pstUVI)			(&(stUVIArray[pstUVI->iPreInfo]))
#define NEXT_INFO(pstUVI)			(&(stUVIArray[pstUVI->iNextInfo]))
#define CURRENT_INFO				(&(stUVIArray[m_CurrentInfoIndex]))

#define DATA_FRONT_INDEX(VAR_INFO)	(VAR_INFO->DataIndex)
#define DATA_END_INDEX(VAR_INFO)	(VAR_INFO->DataIndex + VAR_INFO->ucDataSize)

#define P_DATA(INDEX)				(&(m_pData[INDEX]))

#include <lwrite.h>
#include <string.h>

typedef struct _st_Universal_Variable_Info
{
	int 			iPreInfo, iNextInfo;
	unsigned int	DataIndex;
	unsigned char	ucDataSize;
} st_Universal_Variable_Info, *pst_Universal_Variable_Info;

typedef struct _st_Universal_Variable_Buffer
{
	char			szBody[MAX_UNIVERSAL_VARIABLE_MAX_BODY_LEN];
	unsigned char	ucDataSize;

	void Reset()
	{
		memset(szBody, 0, sizeof(szBody));
		ucDataSize = 0;

		return;
	}

	UCHAR* Push_8(UCHAR data)
	{
		UCHAR* pf = (UCHAR*)(szBody + ucDataSize);
		*pf = data;

		ucDataSize += sizeof(UCHAR);
		return pf;
	}

	USHORT* Push_16(USHORT data)
	{
		USHORT* pf = (USHORT*)(szBody + ucDataSize);
		*pf = data;

		ucDataSize += sizeof(USHORT);
		return pf;
	}

	UINT32* Push_32(UINT32 data)
	{
		UINT32* pf = (UINT32*)(szBody + ucDataSize);
		*pf = data;

		ucDataSize += sizeof(UINT32);
		return pf;
	}

	UINT64* Push_64(UINT64 data)
	{
		UINT64* pf = (UINT64*)(szBody + ucDataSize);
		*pf = data;

		ucDataSize += sizeof(UINT64);
		return pf;
	}

	char* Push_String(int size, char* data)
	{
		if(strlen(data) >= size)
			return NULL;

		char* pf = (char*)(szBody + ucDataSize);

		memset(pf, 0, size);
		memcpy(pf, data, strlen(data));

		ucDataSize += size;
		return pf;
	}

	char* Push_Buffer(int size, char* data)
	{
		char* pf = (char*)(szBody + ucDataSize);
		
		memcpy(pf, data, size);

		ucDataSize += sizeof(char);
		return pf;
	}

	UCHAR Pop_8()
	{
		UCHAR ret = *((UCHAR*)(szBody + ucDataSize));
		ucDataSize += sizeof(UCHAR);

		return ret;
	}

	USHORT Pop_16()
	{
		USHORT ret = *((USHORT*)(szBody + ucDataSize));
		ucDataSize += sizeof(USHORT);

		return ret;
	}

	UINT Pop_32()
	{
		UINT ret = *((UINT*)(szBody + ucDataSize));
		ucDataSize += sizeof(UINT);

		return ret;
	}

	UINT64 Pop_64()
	{
		UINT64 ret = *((UINT64*)(szBody + ucDataSize));
		ucDataSize += sizeof(UINT64);

		return ret;
	}

	char* Pop_String(int size)
	{
		char *ret = (char*)(szBody + ucDataSize);
		ucDataSize += sizeof(size);

		return ret;
	}

} st_Universal_Variable_Buffer, *pst_Universal_Variable_Buffer;

class CUniversalVariableSpace
{
public:
	CUniversalVariableSpace();
	~CUniversalVariableSpace();

	void 							GetData(int iUVIIndex);
	pst_Universal_Variable_Buffer	GetBufferPtr()	{ return &m_stBuffer; }

	int AddData(unsigned char size, char* data);
	int RemoveData(int iUVIIndex);

	int RemoveIndex(pst_Universal_Variable_Info pstUVI);
	int MemoryFree(int moveIndex, pst_Universal_Variable_Info pMoveUVI);
	int SortList(int iUVIIndex);

private:
	st_Universal_Variable_Info	stUVIArray[MAX_UNIVERSAL_VARIABLE_INFO];
	
	char						m_pData[MAX_UNIVERSAL_VARIABLE_SPACE];

	int m_InfoHead, m_InfoTail;
	int m_CurrentInfoIndex;
	int m_InfoCount;

	st_Universal_Variable_Buffer	m_stBuffer;
};

#endif