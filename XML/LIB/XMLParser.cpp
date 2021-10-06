#include <XMLParser.h>

XMLParser::XMLParser(/* args */){}
XMLParser::~XMLParser(){}

int XMLParser::SetXmlPath(char* targetXml)
{
	memset(FILE_PATH + DIR_PATH_LEN, 0, sizeof(FILE_PATH) - DIR_PATH_LEN);
	memcpy(FILE_PATH + DIR_PATH_LEN, targetXml, strlen(targetXml));
	// printf("=== XML Path : %s\n", FILE_PATH);
	return OpenXmlFile(FILE_PATH);
}


int XMLParser::PrintXML(Syntex syntex)
{
	printf("%d", m_syntexlevel);
	for(int i=0; i<m_syntexlevel; i++)
	{
		printf("\t");
	}
	switch (syntex.type)
	{
		case OPEN:
		{
			printf("[%s]\n", syntex.value);
		}	break;
		case CLOSE:
		{
			printf("[/%s]\n", syntex.value);
		}	break;
		case VALUE:
		{
			printf("- %s\n", syntex.value);
		}	break;
		default:
			break;
	}

}

inline int XMLParser::OpenXmlFile(char* filePath) // 상대 경로
{
	if(m_fileInputStream.is_open()){
		return -1;
	}

	m_fileInputStream.open(filePath, ifstream::in); // read only

	if(!m_fileInputStream.is_open()){
		printf(">>>>>>>>> OPEN XML FAIL : %s  <<<<\n", filePath);
		return -2;
	}

	m_readCount = 0;
	m_syntexlevel = 0;
	m_fileInputStream.seekg(0, m_fileInputStream.end);
	m_fileLength = m_fileInputStream.tellg();
	m_fileInputStream.seekg(0, m_fileInputStream.beg);

	printf("<<<<  Open XML File : %s [len : %d]  >>>>\n", filePath, m_fileLength);

	return 0;
}

inline int XMLParser::ReadNext()
{
	// printf(">>> read count : %d / %d\n", m_readCount, m_fileLength);
	if(m_readCount >= m_fileLength) return -1; // end of file

	int readSize;
	if(m_readCount + XML_READ_BUFFER_SIZE > m_fileLength) readSize = m_fileLength - m_readCount;
	else readSize = XML_READ_BUFFER_SIZE;

	if(!m_fileInputStream.read(buffer, readSize))
	{
		printf("Read Err ReadSize : %d\n", readSize);
		return -2; // Read Err
	}
	m_readCount += XML_READ_BUFFER_SIZE;

	SetBuffer(buffer, XML_READ_BUFFER_SIZE);

	return 0;
}

int XMLParser::GetNextValueGroup(Value** ppValueGroup)
{
	bool PrintXMLData = false; //// LOG PRINTS <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

	if(m_fileInputStream)
	{
		int res;
		int valueCount = 0;
		Syntex currentSyntex;
		while(true)
		{
			res = GetNextSyntex(&currentSyntex);
			if(res < 0)
			{
				printf("get syntex error [res:%d]\n", res);
				return res;
			}

			// log
			// printf(" + Type %d\n", currentSyntex.type);
			switch (currentSyntex.type)
			{
				case OPEN:
				{
					if(PrintXMLData) PrintXML(currentSyntex); ////
					m_syntexlevel++;
					m_values[valueCount].name = currentSyntex.value;
				}	break;
				case CLOSE:
				{
					m_syntexlevel--;
					if(PrintXMLData) PrintXML(currentSyntex); ////
					if(m_syntexlevel == 1)
					{
						(*ppValueGroup) = m_values;
						return valueCount;
					}
					else if(m_syntexlevel == 0)
					{
						// printf("XML Read Complete\n");
						return -1;
					}
				}	break;
				case VALUE:
				{
					if(PrintXMLData) PrintXML(currentSyntex); ////
					if(m_syntexlevel != 3) return -4;
					m_values[valueCount].value = currentSyntex.value;
					// log
					// printf("   └ %s \t: %s\n", m_values[valueCount].name, m_values[valueCount].value);
					valueCount++;
					
				}	break;
				case REMARK:
				case HEADER:
					break;
				default:
				{
					printf("[Type Err] %d\n", currentSyntex.type);
					return currentSyntex.type;
				}	break;
			}
		}
		return -3; // syntex error
	}
	else
	{
		return -2; // file not open
	}
}

int XMLParser::SetBuffer(char* buffer, int bufferLenght)
{
	return m_lexer.SetBuffer(buffer, bufferLenght);
}

int XMLParser::GetNextSyntex(Syntex* syntex)
{
	int res;

	syntex->type = SYNTEX_ERROR;
	syntex->value = NULL;

	while(true){

		// '<'가 아니면 읽는다.
		if(nextToken.type != LQ)
		{
			res = GetNextToken(&nextToken);
			if(res == -1) {
				return -1;
			}
		}

		if(syntex->type == SYNTEX_ERROR)	// 문맥 파악
		{
			preToken = nextToken;

			if(nextToken.type == LQ)
			{
				res = GetNextToken(&nextToken);
				if(res == -1) {
					return -1;
				}

				switch (nextToken.type)
				{
				case SLICE_WORD:
				case WORD:
					syntex->type = OPEN;
					break;
				case SLASH:
					syntex->type = CLOSE;
					res = GetNextToken(&nextToken);
					if(res == -1) {
						return SYNTEX_ERROR;
					}
					break;
				case EM:
					syntex->type = REMARK;
					res = GetNextToken(&nextToken);
					if(res == -1) {
						return SYNTEX_ERROR;
					}
					break;
				case QM:
					syntex->type = HEADER;
					res = GetNextToken(&nextToken);
					if(res == -1) {
						return SYNTEX_ERROR;
					}
					break;
				default:
					return SYNTEX_ERROR;
				}
			}
			else
			{
				syntex->type = VALUE;
			}
		}
		else	// 문맥 파악 완료
		{
			// log
			// printf("Syntex : %d\n", syntex->type);
			switch (syntex->type)
			{
			case VALUE:
				if(nextToken.type != LQ)
				{
					if(nextToken.type == DEFAULT) continue;
					CombineWord(&preToken, &nextToken);
				}
				else
				{
					syntex->value = preToken.string;
					return syntex->type;
				}

				break;
			case OPEN:
			case CLOSE:
				if(nextToken.type == SLICE_WORD)
				{
					CombineWord(&preToken, &nextToken);
				}
				else if(nextToken.type == RQ)
				{
					if(syntex->value == NULL) syntex->value = preToken.string; 
					return syntex->type;
				}
				else
				{
					if(syntex->value == NULL) syntex->value = preToken.string; 
				}
				break;
			case REMARK:
			case HEADER: // 현재 해더처리 하지 않음
				if(nextToken.type == RQ)
				{
					return syntex->type;
				}
				break;
			default:
				break;
			}
		}

		preToken = nextToken;
	}
	return -2; // critical
}

inline int XMLParser::GetNextToken(Token* pToken)
{
	int res;
	res = m_lexer.GetNextToken(pToken);
	if(res < 0) {
		if(ReadNext()<0) return -1;
		return GetNextToken(pToken);
	}
	// log
	// if(res == WORD)
	// 	printf(" Next Token = %d [%s]\n", res, pToken->string);
	// else
	// 	printf(" Next Token = %d\n", res);
	return res;
}

// 단어가 버퍼 끝에 딱 맞춰서 끝난 경우 떨어져야 할 단어가 붙을 수 도 있음.  // ex) one two three => onetwo three
int XMLParser::CombineWord(Token* left, Token* right)
{
	switch (right->type)
	{
	case RQ:
		/* code */
		break;
	case LQ:
		return SYNTEX_ERROR;
	default:
		break;
	}

	if(left->type == SLICE_WORD)
	{
		int length = left->length + right->length;
		char* combine = new char[length +1];

		memcpy(combine ,left->string, left->length);
		memcpy((combine + left->length), right->string, right->length); // 마지막 '\0' 까지 복사
		
		// log
		// printf("\tCombine SLICE [%s](%d)+[%s](%d) = [%s](%d)\n", left->string, left->length, right->string, right->length, combine, length);

		right->string = combine;
		right->length = length;
	}
	else if(left->type == WORD)
	{
		int length = left->length + right->length;
		char* combine = new char[length + 2];

		memcpy(combine ,left->string, left->length);
		combine[left->length] = ' ';
		memcpy((combine + left->length + 1), right->string, right->length);

		// log
		// printf("\tCombine [%s](%d)+[%s](%d) = [%s](%d)\n", left->string, left->length, right->string, right->length, combine, length + 1);

		right->string = combine;
		right->length = length + 1;
	}
	else
	{
		return -1;
	}

	return 0;
}