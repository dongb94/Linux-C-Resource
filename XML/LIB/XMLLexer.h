#ifndef __XML_LEXER__
#define __XML_LEXER__

/// LQ WORD ~~~ RQ				== OPEN
/// LQ SLASH WORD RQ			== CLOSE
/// LQ EM DD WORD DD RQ			== REMARK
///	LQ QM ~~~ RQ				== HEADER

/// RQ WORD			== value
/// RQ LQ			== struct

#include <stdio.h>
#include <string.h>

#define XML_READ_BUFFER_SIZE	65535
#define READ_NEXT_SIGNAL		2

enum TokenType
{
	DEFAULT,

	LQ,			// '<'
	RQ,			// '>'
	SLASH,		// '/'
	EM,			// '!'
	QM,			// '?'
	DASH,		// '-'
	DD,			// "--"
	WORD,

	SLICE_WORD
};

struct Token
{
	TokenType	type;
	char*		string;
	unsigned int	length;
};

class XMLLexer
{
private:
	unsigned int m_pointer;
	unsigned int m_bufferSize;
	char m_buffer[XML_READ_BUFFER_SIZE * 2];
	char m_tempBuffer[650];

	inline bool CheckDoubleDash();
	inline char GetWord();
public:
	XMLLexer(/* args */);
	~XMLLexer();

	int SetBuffer(char* buffer, unsigned int bufferSize);
	int GetNextToken(Token* pToken);
};

#endif