#ifndef __XML_PARSER__
#define __XML_PARSER__

#define MAKE_SHM_KEY_FROM_INDEX(index) (((unsigned long long)index)<<48)+1

#define XML_READ_BUFFER_SIZE	500
#define XML_MAX_NUMBER_OF_VALUE	50

#define MONSTER_BASE_DATA_XML_PATH "./MonsterData.xml"
#define MONSTER_PLACEMENT_DATA_XML_PATH "./MonsterPlacementData.xml"
#define MAP_LIST_DATA_XML_PATH "./MapListData.xml"
#define MAP_DATA_XML_PATH "./MapData.xml"
extern char FILE_PATH[512];
extern short DIR_PATH_LEN;

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