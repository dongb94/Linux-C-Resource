#include "QuestData.h"
#include <ElgradoDefine.h>

QuestData::QuestData()
{
	int res;

	Init_hashed_shm_handle(&m_smXMLData, QUEST_XML_SIZE, sizeof(QUEST_XML_DATA), QUEST_XML_SIZE, S_SSHM_XML_QUEST_DATA);
	res = Init_hashed_shm(&m_smXMLData);
	if(res <0)
	{
		//errr
		printf("[QuestData] InitSharedMemory Fail %d\n", res);
	}
}
QuestData::~QuestData(){}

int QuestData::SetXmlData()
{
	int res;
	Value* valueList;

	res = m_parser.SetXmlPath(QUEST_DATA_XML_PATH);
	if(res <0)
	{
		//errr
		printf("set 'QUEST_DATA_XML_PATH' xml path err %d\n", res);
		return res;
	}

	QuestActionData subdata;
	QUEST_XML_DATA QuestData;

	int i=0;
	int index = 0;
	while(true)
	{
		res = m_parser.GetNextValueGroup(&valueList);
		if(res < 0)
		{
			if(res == EOF)
			{
				printf(">>>> COMPLETE READ XML\n");
				return 0;
			}
			printf("====== GetNextValueFalse %d\n", res);
			return -1;
		}

		// printf("valueList length : %d\n", res);
		for(int j=0; j<res; j++)
		{
			if(!strcmp(valueList[j].name,"KEY"))						ConvertToInt(valueList[j].value, &QuestData.Key);
			else if(!strcmp(valueList[j].name,"QuestActionCount"))		ConvertToInt(valueList[j].value, &QuestData.ucQuestActionCount);
			else if(!strcmp(valueList[j].name,"QuestAction"))			ConvertToIntegerArray(valueList[j].value, QuestData.uiQuestAction);
			else if(!strcmp(valueList[j].name,"PreQuest"))				ConvertToIntegerArray(valueList[j].value, QuestData.uiPrevQuest);
			else if(!strcmp(valueList[j].name,"NextQuest"))				ConvertToIntegerArray(valueList[j].value, QuestData.uiNextQuest);
			else if(!strcmp(valueList[j].name,"Reward"))				ConvertToInt(valueList[j].value, &QuestData.uiReward);
			else if(!strcmp(valueList[j].name,"AppropriateLevel"))		ConvertToInt(valueList[j].value, &QuestData.usAppropriateLevel);	//제한 레벨
			else if(!strcmp(valueList[j].name,"QuestName"));
			else if(!strcmp(valueList[j].name,"QuestType"));
			else 														printf("====== Variable name wrong : %s [%s]\n", valueList[j].name, valueList[j].value);
		}

		//printf(" •••• Quest Data [%d]%d\n", i, QuestData.Key);

		UINT64 ullQuestTableKey = MAKE_SHM_KEY_FROM_INDEX(i);

		QUEST_XML_DATA* questXMLData;

		res = Get_hashed_shm(&m_smXMLData, ullQuestTableKey, (void**)&questXMLData);
		if(res < 0)
		{
			res = New_hashed_shm(&m_smXMLData, ullQuestTableKey, (void**)&questXMLData);
			if(res < 0)
			{
				printf("New Hash shm Error [res : %d][ullKey : %llx]\n", res, ullQuestTableKey);
				return -1;
			}
		}

		memcpy(questXMLData, &QuestData, sizeof(QUEST_XML_DATA));
		i++;

		index = subdata.SetXmlData(QuestData.Key, index);
		if(index < 0)
		{
			printf(">>>>>>>>>>>>>>>>READ SUB DATA ERROR <<<<<<<<<<<< [serial %d][index %d]\n", QuestData.Key, index);
			return -1;
		}
		
	}
}