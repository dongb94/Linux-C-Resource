#include <XMLLexer.h>

XMLLexer::XMLLexer(/* args */){}
XMLLexer::~XMLLexer(){}

int XMLLexer::ResetBuffer()
{
	memset(m_buffer, 0 ,sizeof(m_buffer));
	m_pointer = 0;
}

int XMLLexer::SetBuffer(char* buffer, unsigned int bufferSize)
{
	memcpy(m_buffer, m_buffer + XML_READ_BUFFER_SIZE, XML_READ_BUFFER_SIZE);
	memcpy(m_buffer + XML_READ_BUFFER_SIZE, buffer, bufferSize);
	
	if(m_pointer >= XML_READ_BUFFER_SIZE) m_pointer -= XML_READ_BUFFER_SIZE;
	
	return 0;
}

int XMLLexer::GetNextToken(Token* pToken)
{
	if(m_buffer == NULL) return EOF;
	if(m_pointer >= XML_READ_BUFFER_SIZE)
	{
		return -READ_NEXT_SIGNAL;
	}

	while(true)
	{
		char nextChar = m_buffer[m_pointer];
		m_pointer++;

		switch (nextChar)
		{
		case ' ':
		case '\n':
		case '\t':
		case '\r':
		case '\0':
			break;
		case EOF:
			return EOF;
		case '<':
		{
			pToken->type = LQ;
			pToken->string = new char[2];
			pToken->string[0] = '<';
			pToken->string[1] = '\0';
			return LQ;
		}
		case '>':
		{
			pToken->type = RQ;
			pToken->string = new char[2];
			pToken->string[0] = '>';
			pToken->string[1] = '\0';
			pToken->length = 1;
			return RQ;
		}
		case '!':
		{
			pToken->type = EM;
			pToken->string = new char[2];
			pToken->string[0] = '!';
			pToken->string[1] = '\0';
			pToken->length = 1;
			return EM;

		}
		case '?':
		{
			pToken->type = QM;
			pToken->string = new char[2];
			pToken->string[0] = '?';
			pToken->string[1] = '\0';
			pToken->length = 1;
			return QM;

		}	
		case '/':
		{
			pToken->type = SLASH;
			pToken->string = new char[2];
			pToken->string[0] = '/';
			pToken->string[1] = '\0';
			pToken->length = 1;
			return SLASH;

		}
		case '-':
			if(CheckDoubleDash())
			{
				// '--'
				m_pointer++;
				pToken->type = DD;
				pToken->string = new char[3];
				pToken->string[0] = '-';
				pToken->string[1] = '-';
				pToken->string[2] = '\0';
				pToken->length = 2;
				return DD;
			}
			else
			{
				// '-'
				pToken->type = DASH;
				pToken->string = new char[2];
				pToken->string[0] = '-';
				pToken->string[1] = '\0';
				pToken->length = 1;
				return DASH;
			}
		default:	// WORD
			pToken->type = WORD;
			m_tempBuffer[0] = nextChar;
			char length = GetWord();

			pToken->string = new char[length+1];
			memcpy(pToken->string, m_tempBuffer, length);
			pToken->string[length] = '\0';

			pToken->length = length;

			return WORD;
		}
	}

	return EOF;
}

inline bool XMLLexer::CheckDoubleDash(){
	return m_buffer[m_pointer] == '-';
}

inline char XMLLexer::GetWord(){
	unsigned char length = 1; // 0번지가 채워진 상태로 온다.
	while(true)
	{
		char nextChar = m_buffer[m_pointer];
		switch (nextChar)
		{
		case ' ':
		case '\n':
		case '\t':
		case '\r':
		case '\0':
		case '<':
		case '>':
		case '!':
		case '?':
		case '/':
		case '-':
		case EOF:
			return length;
		default:
			m_tempBuffer[length] = nextChar;
			m_pointer++;
			length++;
			break;
		}
	}
	
	return -1;
}