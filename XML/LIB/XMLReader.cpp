#include <XMLReader.h>

XMLReader::XMLReader(){};
XMLReader::~XMLReader(){};

int XMLReader::ConvertToInt(char* string, char* result)
{
	int res;
	long long nCR;
	res = ConvertToInt(string, &nCR);
	*result = (char)nCR;
	return res;
}

int XMLReader::ConvertToInt(char* string, unsigned char* result)
{
	int res;
	unsigned long long nCR;
	res = ConvertToInt(string, &nCR);
	*result = (unsigned char)nCR;
	return res;
}

int XMLReader::ConvertToInt(char* string, short* result)
{
	int res;
	long long nCR;
	res = ConvertToInt(string, &nCR);
	*result = (short)nCR;
	return res;
}

int XMLReader::ConvertToInt(char* string, unsigned short* result)
{
	int res;
	unsigned long long nCR;
	res = ConvertToInt(string, &nCR);
	*result = (unsigned short)nCR;
	return res;
}

int XMLReader::ConvertToInt(char* string, int* result)
{
	int res;
	long long nCR;
	res = ConvertToInt(string, &nCR);
	*result = (int)nCR;
	return res;
}

int XMLReader::ConvertToInt(char* string, unsigned int* result)
{
	int res;
	unsigned long long nCR;
	res = ConvertToInt(string, &nCR);
	*result = (unsigned int)nCR;
	return res;
}

int XMLReader::ConvertToInt(char* string, long long* result)
{
	
	bool isMinus = false;
	int count=0;
	char c;
	*result = 0;
	while(true)
	{
		c = string[count];
		if(c == '-')
		{
			isMinus = true;
		}
		if(c >= '0' && c <= '9')
		{
			break;
		}
		count++;

		if(count > strlen(string))
		{
			printf("[convert error] %s\n", string);
			return -1;
		}
	}
	while(true)
	{
		c = string[count];
		// printf("[%d]<%c:%d> (%d),\n" ,count, c,c, *result);
		if(c >= '0' && c <= '9')
		{
			*result *= 10;
			*result += c - '0';
		}
		else
		{
			if(isMinus) *result = -(*result);
			return count;
		}

		count++;
	}

	return 0;
}

int XMLReader::ConvertToInt(char* string, unsigned long long* result)
{
	int count=0;
	char c;
	*result = 0;
	while(true)
	{
		c = string[count];
		if(c >= '0' && c <= '9')
		{
			break;
		}
		count++;

		if(count > strlen(string))
		{
			printf("[convert error] %s\n", string);
			return -1;
		}
	}
	while(true)
	{
		c = string[count];
		// printf("[%d]<%c:%d> (%d),\n" ,count, c,c, *result);
		if(c >= '0' && c <= '9')
		{
			*result *= 10;
			*result += c - '0';
		}
		else
		{

			return count;
		}

		count++;
	}

	return 0;
}

int XMLReader::ConvertToFloat(char* string, float* result)
{
	int count = 0;
	*result = (float)atof(string);
	return count;
}

int XMLReader::ConvertToFloat(char* string, double* result)
{
	int count = 0;
	*result = atof(string);
	return count;
}

int XMLReader::ConvertToIntVecter(char* string, int* x, int* y, int* z)
{
	int res;
	int offset = 0;
	res = ConvertToInt(string, x);
	if(res < 0) {
		printf("Conver To Int Err 1 |%d| %llx \n",offset, string);
		return 0;
	}
	offset = res;

	res = ConvertToInt(&string[offset], y);
	if(res < 0) {
		printf("Conver To Int Err 2 |%d| %llx \n",offset, (&string[offset]));
		return 0;
	}
	offset += res;

	res = ConvertToInt(&string[offset], y);
	if(res < 0) {
		printf("Conver To Int Err 3 |%d| %llx \n",offset, (&string[offset]));
		return 0;
	}
	offset += res;

	return 0;
}

int XMLReader::ConvertToIntVecter(char* string, unsigned* x, unsigned* y, unsigned* z)
{
	int res;
	int offset = 0;
	res = ConvertToInt(string, x);
	if(res < 0) {
		printf("Conver To Int Err 1 |%d| %llx \n",offset, string);
		return 0;
	}
	offset = res;

	res = ConvertToInt(&string[offset], y);
	if(res < 0) {
		printf("Conver To Int Err 2 |%d| %llx \n",offset, (&string[offset]));
		return 0;
	}
	offset += res;

	res = ConvertToInt(&string[offset], y);
	if(res < 0) {
		printf("Conver To Int Err 3 |%d| %llx \n",offset, (&string[offset]));
		return 0;
	}
	offset += res;

	return 0;
}

int XMLReader::ConvertToFloatVecter(char* string, float* x, float* y, float* z)
{
	int offset = 0;
	ConvertToFloat(string, x);
	while(true)
	{
		char c = string[offset];
		if(c ==',') break;

		if(c >= '0' && c <= '9');
		else if(c=='.' || c==' ' || c=='-');
		else // wrong input string
			return -1;

		offset++;
	}

	ConvertToFloat((char*)(&string[offset]), y);
	while(true)
	{
		char c = string[offset];
		if(c ==',') break;

		if(c >= '0' && c <= '9');
		else if(c=='.' || c==' ' || c=='-');
		else // wrong input string
			return -1;
		
		offset++;
	}

	ConvertToFloat((char*)(&string[offset]), z);
}

int XMLReader::ConvertToIntegerArray(char* string, void* array, int arraySize, int maxSize)
{
	int startOffset = 0;
	int offset = 0;
	int count = 0;

	// printf("ORIGINAL DATA : %s\n", string);
	while(count < maxSize)
	{
		char c = string[offset];
		if(c=='\0')
		{
			array = (char*)array+(count * arraySize);
			if(arraySize < 5)
			{
				*((int *)array) = atoi(&string[startOffset]);

				// printf("\t %d %d\n", atoi(&string[startOffset]), *((int*)(array)));
			}
			else
			{
				*((long long *)array) = atoll(&string[startOffset]);
			}
			break;
		}
		else if(c==',') {
			string[offset] = '\0';

			array = (char*)array+(count * arraySize);
			if(arraySize < 5)
			{
				*((int *)array) = atoi(&string[startOffset]);

				// printf("\t %d %d\n", atoi(&string[startOffset]), *((int*)(array)));
			}
			else
			{
				*((long long *)array) = atoll(&string[startOffset]);
			}
			count++;
			startOffset = offset+1;
		}

		offset++;
	}
}