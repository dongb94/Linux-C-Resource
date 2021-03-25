
#ifndef ___XML______
#define ___XML______

#include <hashshm.h>
#include <lwrite.h>

#include <GameStructer.h>

class XML
{
public :
	int SetXMLTarget(unsigned int uiEntitySize, unsigned int uiEntityNum, unsigned long long shmKey);

	int At(unsigned long long index, XML_BASE **ppData);
	int GetXMLData(unsigned long long dataKey, XML_BASE **ppData);

private :
	unsigned int maxSize;
	st_HashedShmHandle m_pHandle;
};


#endif // !___XML______