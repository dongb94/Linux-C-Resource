
#include <unistd.h>
#include <stdlib.h>

#include "MapData.h"
#include "DropData.h"
#include "DropSetData.h"
#include "ReinforceData.h"
#include "DestroyData.h"
#include "DismantleData.h"
#include "RandomOptionData.h"
#include "CombineJewerlyData.h"
#include "MaterialData.h"
#include "QuestData.h"
#include "BuffData.h"

char FILE_PATH[512];
short DIR_PATH_LEN;

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

    MapData mapData;
    DropData dropData;
    DropSetData dropSetData;
    ReinforceData reinforceSetData;
    DestroyData destroyData;
    DismantleData dismantleData;
    RandomOptionData randomOptionData;
    CombineJewerlyData combineJewerlyData;
    MaterialData materialData;
    QuestData questData;
    BuffData buffData;

    mapData.SetXmlData();
    dropData.SetXmlData();
    dropSetData.SetXmlData();
    reinforceSetData.SetXmlData();
    destroyData.SetXmlData();
    dismantleData.SetXmlData();
    randomOptionData.SetXmlData();
    combineJewerlyData.SetXmlData();
    materialData.SetXmlData();
    questData.SetXmlData();
    buffData.SetXmlData();

    return 0;
}