#ifndef __XML_READER__
#define __XML_READER__

// BDG 21.02.02 Add convert Funcs

#include <hashshm.h>
#include <shmutil.h>
#include <linkshm.h>

#include <GameStructer.h>

#include <SharedMemory.h>

#include <XMLParser.h>

// 공유메모리에 정보가 없으면 -> xml 읽기 -> 공유메모리에 적제 -> xml 닫기

class XMLReader
{
public :
	XMLReader();
	~XMLReader();

	int ConvertToInt(char* string, char* result);
	int ConvertToInt(char* string, unsigned char* result);
	int ConvertToInt(char* string, short* result);
	int ConvertToInt(char* string, unsigned short* result);
	int ConvertToInt(char* string, int* result);
	int ConvertToInt(char* string, unsigned int* result);
	int ConvertToInt(char* string, long long* result);
	int ConvertToInt(char* string, unsigned long long* result);

	int ConvertToFloat(char* string, float* result);
	int ConvertToFloat(char* string, double* result);

	int ConvertToIntVecter(char* string, int* x, int* y, int* z);
	int ConvertToIntVecter(char* string, unsigned* x, unsigned* y, unsigned* z);
	int ConvertToFloatVecter(char* string, float* x, float* y, float* z);

	int ConvertToIntegerArray(char* string, void* array, int arraySize = sizeof(int), int maxSize = 255);

protected : 
	virtual int SetXmlData() = 0;
	
	st_HashedShmHandle m_smXMLData;

	XMLParser m_parser;
};

#endif