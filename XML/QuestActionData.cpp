#include "QuestActionData.h"
#include <ElgradoDefine.h>

QuestActionData::QuestActionData()
{
	int res;

	Init_hashed_shm_handle(&m_smXMLData, QUEST_ACTION_XML_SIZE, sizeof(QUEST_ACTION_XML_DATA), QUEST_ACTION_XML_SIZE, S_SSHM_XML_QUEST_ACTION_DATA);
	res = Init_hashed_shm(&m_smXMLData);
	if(res <0)
	{
		//errr
		printf("[QuestActionData] InitSharedMemory Fail %d\n", res);
	}
}
QuestActionData::~QuestActionData(){}

int QuestActionData::SetXmlData(int serial, int index)
{
	int res;
	Value* valueList;

	char filePath[256];
	sprintf(filePath, "%s%d.xml", QUEST_ACTION_DATA_XML_PATH, serial);

	res = m_parser.SetXmlPath(filePath);
	if(res <0)
	{
		//errr
		printf("set 'QUEST_ACTION_DATA_XML_PATH' xml path err %d [path %s]\n", res, filePath);
		return res;
	}

	QUEST_ACTION_XML_DATA QuestActionData;

	int i=index;
	while(true)
	{
		res = m_parser.GetNextValueGroup(&valueList);
		if(res < 0)
		{
			if(res == EOF)
			{
				printf(">>>> COMPLETE READ XML\n");
				return i;
			}
			printf("====== GetNextValueFalse %d\n", res);
			return -1;
		}

		printf("valueList length : %d\n", res);
		for(int j=0; j<res; j++)
		{
			if(!strcmp(valueList[j].name,"KEY"))						ConvertToInt(valueList[j].value, &QuestActionData.Key);
			else if(!strcmp(valueList[j].name,"Sequence"))				ConvertToInt(valueList[j].value, &QuestActionData.usQuestSequence);
			else if(!strcmp(valueList[j].name,"ActionType"))			ConvertToInt(valueList[j].value, &QuestActionData.usQuestActionType);
			else if(!strcmp(valueList[j].name,"TargetIndexList"))		ConvertToIntegerArray(valueList[j].value, QuestActionData.ullTargetList);
			else if(!strcmp(valueList[j].name,"GoalCount"))				ConvertToInt(valueList[j].value, &QuestActionData.uiGoalCount);
			else if(!strcmp(valueList[j].name,"QuestSubTitle"));
			else if(!strcmp(valueList[j].name,"TalkTableIndex"));
			else if(!strcmp(valueList[j].name,"TargetScene"));
			else if(!strcmp(valueList[j].name,"TargetDistance"));
			else if(!strcmp(valueList[j].name,"TargetPosition"));
			else 														printf("====== Variable name wrong : %s [%s]\n", valueList[j].name, valueList[j].value);
		}

		//printf(" •••• Quest Action Data [%d]%d\n", i, QuestActionData.Key);

		UINT64 ullQuestActionTableKey = MAKE_SHM_KEY_FROM_INDEX(i);

		QUEST_ACTION_XML_DATA* questActionXMLData;

		res = Get_hashed_shm(&m_smXMLData, ullQuestActionTableKey, (void**)&questActionXMLData);
		if(res < 0)
		{
			res = New_hashed_shm(&m_smXMLData, ullQuestActionTableKey, (void**)&questActionXMLData);
			if(res < 0)
			{
				printf("New Hash shm Error [res : %d][ullKey : %llx]\n", res, ullQuestActionTableKey);
				return -1;
			}
		}

		memcpy(questActionXMLData, &QuestActionData, sizeof(QUEST_ACTION_XML_DATA));
		i++;
	}


}