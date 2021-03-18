#include "MapData.h"
#include <ElgradoDefine.h>

MapData::MapData()
{
	int res;

	Init_hashed_shm_handle(&m_smXMLData, MAP_XML_SIZE, sizeof(MAP_XML_DATA), MAP_XML_SIZE, S_SSHM_XML_MAP_DATA);
	res = Init_hashed_shm(&m_smXMLData);
	if(res <0)
	{
		//errr
		printf("InitSharedMemory Fail %d\n", res);
	}
}
MapData::~MapData(){}

int MapData::SetXmlData()
{
	int res;
	Value* valueList;

	res = m_parser.SetXmlPath(MAP_DATA_XML_PATH);
	if(res <0)
	{
		//errr
		printf("set 'MAP_DATA_XML_PATH' xml path err %d\n", res);
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

		MAP_XML_DATA mapData;

		printf("valueList length : %d\n", res);
		for(int i=0; i<res; i++)
		{
			if(!strcmp(valueList[i].name,"KEY"))						ConvertToInt(valueList[i].value, &mapData.Key);
			else if(!strcmp(valueList[i].name,"floor"))					ConvertToInt(valueList[i].value, &mapData.floor);
			else if(!strcmp(valueList[i].name,"mapName"))				strcpy(mapData.mapName, valueList[i].value);
			else if(!strcmp(valueList[i].name,"worldPosition"))			ConvertToIntVecter(valueList[i].value, &mapData.worldX, &mapData.worldY, &mapData.worldZ);
			else if(!strcmp(valueList[i].name,"warpPosition"))			;
			else 														printf("====== Variable name wrong : %s [%s]\n", valueList[i].name, valueList[i].value);
		}

		printf("mapData key : %d\n", mapData.Key);
		//Build_hashed_shm_key(&m_smXMLData, mapData.Key);

		UINT64 ullMonsterKey = MAKE_SHM_KEY_FROM_INDEX(mapData.Key);
		MAP_XML_DATA* mapXMLData;

		res = Get_hashed_shm(&m_smXMLData, ullMonsterKey, (void**)&mapXMLData);
		if(res < 0)
		{
			res = New_hashed_shm(&m_smXMLData, ullMonsterKey, (void**)&mapXMLData);
			if(res < 0)
			{
				printf("New Hash shm Error [res : %d][ullKey : %llx]", res, ullMonsterKey);
				return -1;
			}
		}

		mapXMLData->Key = mapData.Key;
		mapXMLData->floor = mapData.floor;
		memcpy(mapXMLData->mapName, mapData.mapName, 19);
		mapXMLData->worldX = mapData.worldX;
		mapXMLData->worldY = mapData.worldY;
		mapXMLData->worldZ = mapData.worldZ;

	}
}