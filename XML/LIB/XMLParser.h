#ifndef __XML_PARSER__
#define __XML_PARSER__

#ifndef MAKE_SHM_KEY_FROM_INDEX
#define MAKE_SHM_KEY_FROM_INDEX(index) (((unsigned long long)index)<<48)+1
#endif

#define XML_MAX_NUMBER_OF_VALUE	50

#define MONSTER_BASE_DATA_XML_PATH "./MonsterData.xml"
#define MONSTER_PLACEMENT_DATA_XML_PATH "./MonsterPlacementData.xml"
#define MAP_LIST_DATA_XML_PATH "./MapListData.xml"
#define MAP_DATA_XML_PATH "./MapData.xml"
#define DROP_DATA_XML_PATH "./DropData.xml"
#define DROP_SET_DATA_XML_PATH "./DropSetData.xml"
#define REINFORCE_DATA_XML_PATH "./ReinforceData.xml"
#define MATERIAL_DATA_XML_PATH "./MaterialData.xml"
#define DISMANTLE_DATA_XML_PATH "./DismantleData.xml"
#define RANDOM_OPTION_DATA_XML_PATH "./RandomOptionData.xml"
#define COMBINE_JEWERLY_DATA_XML_PATH "./CombineJewerlyData.xml"
#define COMBINE_SYNERGY_ITEM_DATA_XML_PATH "./CombineSynergyItemData.xml"
#define BUFF_DATA_XML_PATH "./BuffData.xml"
#define QUEST_DATA_XML_PATH "./QuestData.xml"
#define QUEST_ACTION_DATA_XML_PATH "./QuestPlacementData_"
#define REWARD_DATA_XML_PATH "./RewardData.xml"
#define SKILL_BOOK_DATA_XML_PATH "./SkillBookData.xml"
#define RANDOM_PICK_UP_DATA_XML_PATH "./RandomPickUpData.xml"
#define RANDOM_PICK_UP_LIST_DATA_XML_PATH "./RandomPickUpListData.xml"
#define ITEM_PACKAGE_SET_DATA_XML_PATH "./ItemPackageSetData.xml"
extern char FILE_PATH[512];
extern short DIR_PATH_LEN;
extern bool printLog;
extern bool printHardLog;

#include <fstream>
#include <iostream>
#include <XMLLexer.h>

using namespace std;

enum SyntexType
{
    OPEN,
    CLOSE,
    REMARK,
    HEADER,
    VALUE,

	END_OF_FILE = -1,
	UNREACHABLE_LINE = -2,
	SYNTEX_ERROR = -10
};

struct Syntex
{
    SyntexType type;
    char* value;
};

struct Value
{
	char* name;
	char* value;
};

class XMLParser
{
private:
    XMLLexer m_lexer;

	Token preToken;
	Token nextToken;

	ifstream m_fileInputStream;
	unsigned int m_readCount;
	unsigned int m_fileLength;
	char buffer[XML_READ_BUFFER_SIZE];

	int m_syntexlevel;
	Value m_values[XML_MAX_NUMBER_OF_VALUE];

	int PrintXML(Syntex syntex);

	inline int OpenXmlFile(char* filePath);

	inline int ReadNext();		// read next xml file contents to buffer
	int SetBuffer(char* buffer, int bufferLength);

	inline int GetNextToken(Token* token);	// get next token from lexer
	int CombineWord(Token* left, Token* right);

    int GetNextSyntex(Syntex* syntex);

public:
    XMLParser(/* args */);
    ~XMLParser();

	int SetXmlPath(char* targetXml);

    int GetNextValueGroup(Value** ppValueGroup);
};

#endif