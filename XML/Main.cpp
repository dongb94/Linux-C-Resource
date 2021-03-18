
#include <unistd.h>
#include <stdlib.h>

#include "MapData.h"
#include "MapListData.h"
#include "MonsterData.h"
#include "MonsterPlacementData.h"

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
    MapListData mapListData;
    MonsterData monsterData;
    MonsterPlacementData monsterPlacementData;

    mapData.SetXmlData();
    mapListData.SetXmlData();
    monsterData.SetXmlData();
    monsterPlacementData.SetXmlData();

    return 0;
}