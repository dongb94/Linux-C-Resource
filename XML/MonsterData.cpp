#include "MonsterData.h"
#include <ElgradoDefine.h>

MonsterData::MonsterData()
{
	
	int res;

	Init_hashed_shm_handle(&m_smXMLData, MONSTER_XML_SIZE, sizeof(MONSTER_XML_DATA), MONSTER_XML_SIZE, S_SSHM_XML_MONSTER_BASE_DATA);
	res = Init_hashed_shm(&m_smXMLData);
	if(res <0)
	{
		//errr
		printf("InitSharedMemory Fail %d\n", res);
	}
}
MonsterData::~MonsterData(){}

int MonsterData::SetXmlData()
{
	int res;
	Value* valueList;


	res = m_parser.SetXmlPath(MONSTER_BASE_DATA_XML_PATH);
	if(res <0)
	{
		//errr
		printf("set 'MONSTER_BASE_DATA_XML_PATH' xml path err %d\n", res);
		return res;
	}

	while(true)
	{	
		res = m_parser.GetNextValueGroup(&valueList);
		if(res < 0)
		{
			printf("GetNextValueFalse %d\n", res);
			return -1;
		}

		MONSTER_XML_DATA monsterData;

		printf("valueList length : %d\n", res);
		for(int i=0; i<res; i++)
		{
			if(!strcmp(valueList[i].name,"KEY"))						ConvertToInt(valueList[i].value, &monsterData.Key);
			else if(!strcmp(valueList[i].name,"hpBase"))				ConvertToInt(valueList[i].value, &monsterData.HpBase);
			else if(!strcmp(valueList[i].name,"hpRecovery"));
			else if(!strcmp(valueList[i].name,"physicalAttackBase"))	ConvertToInt(valueList[i].value, &monsterData.AttackBase);
			else if(!strcmp(valueList[i].name,"physicalCritRate"));
			else if(!strcmp(valueList[i].name,"attackSpeed"));
			else if(!strcmp(valueList[i].name,"movementSpeed"))			ConvertToInt(valueList[i].value, &monsterData.MovementSpeed);
			else if(!strcmp(valueList[i].name,"physicalDefenseBase"));
			else if(!strcmp(valueList[i].name,"magicDefenseBase"));
			else if(!strcmp(valueList[i].name,"hitRate"));
			else if(!strcmp(valueList[i].name,"dodgeRate"));
			else if(!strcmp(valueList[i].name,"DebuffRegistRate"));
			else 														printf("Variable name wrong : %s [%s]\n", valueList[i].name, valueList[i].value);
		}

		printf("monsterData key : %d\n", monsterData.Key);
		//Build_hashed_shm_key(&m_smXMLData, monsterData.Key);

		UINT64 ullMonsterKey = MAKE_SHM_KEY_FROM_INDEX(monsterData.Key);
		MONSTER_XML_DATA* monsterXMLData;

		res = Get_hashed_shm(&m_smXMLData, ullMonsterKey, (void**)&monsterXMLData);
		if(res < 0)
		{
			res = New_hashed_shm(&m_smXMLData, ullMonsterKey, (void**)&monsterXMLData);
			if(res < 0)
			{
				printf("New Hash shm Error [res : %d][ullKey : %llx]", res, ullMonsterKey);
				return -1;
			}
		}

		monsterXMLData->Key = monsterData.Key;
		monsterXMLData->HpBase = monsterData.HpBase;
		monsterXMLData->AttackBase = monsterData.AttackBase;
		monsterXMLData->MovementSpeed = monsterData.MovementSpeed;

	}
}