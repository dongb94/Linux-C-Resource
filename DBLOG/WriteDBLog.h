#ifndef ___DDDD_BBB_LOG
#define ___DDDD_BBB_LOG

#include <SharedMemoryTree.h>
#include <SharedMemory.h>
#include <ElgradoDefine.h>
#include <typedef.h>

#include <sys/ipc.h>
#include <sys/msg.h>

#define MAX_PACKET_TRAFFIC_DATA_SIZE 2048

typedef struct _PacketTrafficMsgQueue
{
	long long msgType;
	short dataSize;
	short packetCount;
	char pData[MAX_PACKET_TRAFFIC_DATA_SIZE];
} PacketTrafficMsgQueue, *pPacketTrafficMsgQueue;

typedef struct _PacketTrafficData
{
	unsigned short	msgId;
	unsigned short	count;
	unsigned long long	dataSize;
} PacketTrafficData;

void InitDBLog();
int dDBLog(USHORT msgId, USHORT count, UINT32 dataSize);
int AddData(void* pData , int dataSize);
int ResetData();

#endif //!def ___DDDD_BBB_LOG

