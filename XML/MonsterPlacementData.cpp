#include "MonsterPlacementData.h"
#include <ElgradoDefine.h>

MonsterPlacementData::MonsterPlacementData()
{
	int res;

	Init_hashed_shm_handle(&m_smXMLData, MONSTER_SPAWN_XML_SIZE, sizeof(MONSTER_SPAWN_XML_DATA), MONSTER_SPAWN_XML_SIZE, S_SSHM_XML_MONSTER_SPAWN_DATA);
	res = Init_hashed_shm(&m_smXMLData);
	if(res <0)
	{
		//errr
		printf("Init 'MonsterPlacementData' SharedMemory Fail %d\n", res);
	}
}
MonsterPlacementData::~MonsterPlacementData(){}

int MonsterPlacementData::SetXmlData()
{
	int res;
	Value* valueList;


	res = m_parser.SetXmlPath(MONSTER_PLACEMENT_DATA_XML_PATH);
	if(res <0)
	{
		//errr
		printf("set 'MONSTER_PLACEMENT_DATA_XML_PATH' err %d\n", res);
		return res;
	}

	while(true)
	{
		res = m_parser.GetNextValueGroup(&valueList);
		if(res < 0)
		{
			printf("GetNextValueFalse %d\n", res);
			return 0;
		}

		MONSTER_SPAWN_XML_DATA monsterSpawnData;

		printf("valueList length : %d\n", res);
		for(int i=0; i<res; i++)
		{
			if(!strcmp(valueList[i].name,"KEY"))						ConvertToInt(valueList[i].value, &monsterSpawnData.SpawnKey);
			else if(!strcmp(valueList[i].name,"Map"))					ConvertToInt(valueList[i].value, &monsterSpawnData.Map);
			else if(!strcmp(valueList[i].name,"MonsterKey"))			ConvertToInt(valueList[i].value, &monsterSpawnData.MonsterKey);
			else if(!strcmp(valueList[i].name,"Level"))					ConvertToInt(valueList[i].value, &monsterSpawnData.Level);
			else if(!strcmp(valueList[i].name,"Position"))				ConvertToIntVecter(valueList[i].value, &monsterSpawnData.x, &monsterSpawnData.y, &monsterSpawnData.z);
			else if(!strcmp(valueList[i].name,"Rotation"))				ConvertToInt(valueList[i].value, &monsterSpawnData.rotate);
			else 														printf("Variable name wrong : %s [%s]\n", valueList[i].name, valueList[i].value);
		}

		printf("monsterSpawnData [spawn key : %d][monster key : %d]\n", monsterSpawnData.SpawnKey, monsterSpawnData.MonsterKey);

		UINT64 ullMonsterSpawnerKey = MAKE_SHM_KEY_FROM_INDEX(monsterSpawnData.SpawnKey);
		MONSTER_SPAWN_XML_DATA* pMonsterSpawnXMLData;

		res = Get_hashed_shm(&m_smXMLData, ullMonsterSpawnerKey, (void**)&pMonsterSpawnXMLData);
		if(res < 0)
		{
			res = New_hashed_shm(&m_smXMLData, ullMonsterSpawnerKey, (void**)&pMonsterSpawnXMLData);
			if(res < 0)
			{
				printf("New Hash shm Error [res : %d][ullKey : %llx]", res, ullMonsterSpawnerKey);
				return -1;
			}
		}

		pMonsterSpawnXMLData->SpawnKey = monsterSpawnData.SpawnKey;
		pMonsterSpawnXMLData->Map = monsterSpawnData.Map;
		pMonsterSpawnXMLData->MonsterKey = monsterSpawnData.MonsterKey;
		pMonsterSpawnXMLData->Level = monsterSpawnData.Level;
		pMonsterSpawnXMLData->x = monsterSpawnData.x;
		pMonsterSpawnXMLData->y = monsterSpawnData.y;
		pMonsterSpawnXMLData->z = monsterSpawnData.z;
		pMonsterSpawnXMLData->rotate = monsterSpawnData.rotate;
	}
}
