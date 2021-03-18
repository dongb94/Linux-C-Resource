#include "MapListData.h"
#include <ElgradoDefine.h>

MapListData::MapListData()
{
	int res;

	Init_hashed_shm_handle(&m_smXMLData, MAP_LIST_XML_SIZE, sizeof(MAP_LIST_XML_DATA), MAP_LIST_XML_SIZE, S_SSHM_XML_MAP_LIST_DATA);
	res = Init_hashed_shm(&m_smXMLData);
	if(res <0)
	{
		//errr
		printf("InitSharedMemory Fail %d\n", res);
	}
}

MapListData::~MapListData(){}

int MapListData::SetXmlData()
{
	int res;
	Value* valueList;

	res = m_parser.SetXmlPath(MAP_LIST_DATA_XML_PATH);
	if(res <0)
	{
		//errr
		printf("set 'MAP_LIST_DATA_XML_PATH' xml path err %d\n", res);
		return res;
	}

	while(true)
	{
		res = m_parser.GetNextValueGroup(&valueList);
		if(res < 0)
		{
			printf("====== GetNextValueFalse %d\n", res);
			return -1;
		}

		MAP_LIST_XML_DATA mapListData;

		printf("valueList length : %d\n", res);
		for(int i=0; i<res; i++)
		{
			if(!strcmp(valueList[i].name,"KEY"))						ConvertToInt(valueList[i].value, &mapListData.Key);
			else if(!strcmp(valueList[i].name,"mapKey"))				ConvertToInt(valueList[i].value, &mapListData.mapKey);
			else 														printf("====== Variable name wrong : %s [%s]\n", valueList[i].name, valueList[i].value);
		}

		printf("mapListData key : %d\n", mapListData.Key);

		UINT64 ullMapListKey = MAKE_SHM_KEY_FROM_INDEX(mapListData.Key);
		MAP_LIST_XML_DATA* mapListXMLData;


		res = Get_hashed_shm(&m_smXMLData, ullMapListKey, (void**)&mapListXMLData);
		if(res < 0)
		{
			res = New_hashed_shm(&m_smXMLData, ullMapListKey, (void**)&mapListXMLData);
			if(res < 0)
			{
				printf("New Hash shm Error [res : %d][ullKey : %llx]", res, ullMapListKey);
				return -1;
			}
		}

		mapListXMLData->Key = mapListData.Key;
		mapListXMLData->mapKey = mapListData.mapKey;

	}
}