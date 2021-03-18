#include <XMLLexer.h>

XMLLexer::XMLLexer(/* args */){}
XMLLexer::~XMLLexer(){}

int XMLLexer::SetBuffer(char* buffer, unsigned int bufferSize)
{
	m_buffer = buffer;
	m_bufferSize = bufferSize;
	m_pointer = 0;
	return 0;
}

int XMLLexer::GetNextToken(Token* pToken)
{
	if(m_buffer == NULL) return EOF;

	while(m_pointer < m_bufferSize)
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
			pToken->type = LQ;
			return LQ;
		case '>':
			pToken->type = RQ;
			return RQ;
		case '!':
			pToken->type = EM;
			return EM;
		case '?':
			pToken->type = QM;
			return QM;
		case '/':
			pToken->type = SLASH;
			return SLASH;
		case '-':
			if(CheckDoubleDash())
			{
				// '--'
				m_pointer++;
				pToken->type = DD;
				return DD;
			}
			else
			{
				// '-'
				pToken->type = DASH;
				return DASH;
			}
		default:	// WORD
			pToken->type = WORD;
			m_tempBuffer[0] = nextChar;
			char length = GetWord();

			if(length < 0) {
				pToken->type = SLICE_WORD;
				length = -length;
			}

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
	if(m_pointer < m_bufferSize)
		return m_buffer[m_pointer] == '-';
	else
		return false;
}

inline char XMLLexer::GetWord(){
	unsigned char length = 1;
	while(m_pointer < m_bufferSize)
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
			return length;
		case EOF:
			return -length;
		default:
			m_tempBuffer[length] = nextChar;
			m_pointer++;
			length++;
			break;
		}
	}
	
	return length;
}