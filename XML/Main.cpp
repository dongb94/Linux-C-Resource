
#include <unistd.h>
#include <stdlib.h>

#include "MapData.h"
#include "DropData.h"
#include "DropSetData.h"
#include "ReinforceData.h"
#include "DismantleData.h"
#include "RandomOptionData.h"
#include "CombineJewerlyData.h"
#include "CombineSynergyItemData.h"
#include "MaterialData.h"
#include "QuestData.h"
#include "BuffData.h"
#include "RewardData.h"
#include "SkillBookData.h"
#include "RandomPickUpData.h"
#include "RandomPickUpListData.h"

char FILE_PATH[512];
short DIR_PATH_LEN;
bool printLog = false;
bool printHardLog = false;

int main(int argc, char** argv)
{
	if(!realpath(argv[0], FILE_PATH))
	{
		fprintf(stderr, "realpath error\n");
		return -3;
	}

	for(int i=strlen(FILE_PATH)-1; i >= 0; i--)
	{
		if(FILE_PATH[i] == '/')
		{
			DIR_PATH_LEN = i+1;
			break;
		}
	}

	printf("=== EXE Path : %s, DIR LEN : %d\n", FILE_PATH, DIR_PATH_LEN);

	if(argc > 1)
	{
		for(int i = 1; i<argc; i++)
		{
			if(!strcmp("-l", argv[i]))
			{
				printf(" - Print LOG\n");
				printLog = true;
			}
			else if(!strcmp("-hl", argv[i]))
			{
				printf(" - Print LOG\n");
				printHardLog = true;
			}
		}
	}

	MapData mapData;
	DropData dropData;
	DropSetData dropSetData;
	ReinforceData reinforceSetData;
	DismantleData dismantleData;
	RandomOptionData randomOptionData;
	CombineJewerlyData combineJewerlyData;
	CombineSynergyItemData combineSynergyItemData;
	MaterialData materialData;
	QuestData questData;
	BuffData buffData;
	RewardData rewardData;
	SkillBookData skillBookData;
	RandomPickUpData randomPickUpData;
	RandomPickUpListData randomPickUpListData;

	mapData.SetXmlData();
	dropData.SetXmlData();
	dropSetData.SetXmlData();
	dismantleData.SetXmlData();
	combineJewerlyData.SetXmlData();
	combineSynergyItemData.SetXmlData();
	materialData.SetXmlData();
	questData.SetXmlData();
	buffData.SetXmlData();
	reinforceSetData.SetXmlData();
	rewardData.SetXmlData();
	randomOptionData.SetXmlData();
	skillBookData.SetXmlData();
	randomPickUpData.SetXmlData();
	randomPickUpListData.SetXmlData();

	return 0;
}