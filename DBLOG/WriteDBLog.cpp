
#include <WriteDBLog.h>
#include <unistd.h>

int DBlog_qid;

static PacketTrafficMsgQueue m_dataQueue;

static CSharedMemoryTree shmDBLogIndex;
static CSharedMemory shmDBLog;

static PacketTrafficData* logHeader;
static PacketTrafficData* packetData;

void InitDBLog()
{
	int nRet;

	shmDBLogIndex.CreateArray(S_SSHM_DB_LOG_TRAFFIC_INDEX, MAX_KIND_OF_PACKET, sizeof(short));
	shmDBLog.CreateArray(S_SSHM_DB_LOG_TRAFFIC, MAX_KIND_OF_PACKET, sizeof(PacketTrafficData));

	DBlog_qid = msgget(LINUX_MESSAGE_ID_DBLOG, 0666 | IPC_CREAT);
	if(DBlog_qid < 0)
	{
		dAppLog(LOG_CRI, "msgget Error WriteDBLog : %x\n", LINUX_MESSAGE_ID_DBLOG);
		return;
	}

	logHeader = (PacketTrafficData*)shmDBLog[0];
	logHeader->count = 0;
	m_dataQueue.msgType = 119;
}

int dDBLog(USHORT msgId, USHORT count, UINT32 dataSize)
{
	if(count == 0) return 0;
	while(logHeader->msgId == 1) usleep(100);
	short *index = (short*)shmDBLogIndex[msgId];
	dAppLog(LOG_DEBUG, "+DBLOG [%x][%dbyte][x%d]  [LOG_NUM %d]", msgId, dataSize, count, logHeader->count);
	if((*index)==0)
	{
		logHeader->count++;
		packetData = (PacketTrafficData*)shmDBLog[logHeader->count];
		if(packetData == NULL)
		{
			dAppLog(LOG_CRI, "-----DB LOGING ERROR-----get packet traffic data FAIL");
			return -1;
		}
		packetData->msgId = msgId;
		packetData->count = count;
		packetData->dataSize = dataSize * count;
		*index = logHeader->count;
		dAppLog(LOG_DEBUG, "\t[%d][%x][%dbyte][x%d]", *index, msgId, packetData->dataSize, packetData->count);
	}
	else
	{
		packetData = (PacketTrafficData*)shmDBLog[(*index)];
		packetData->count += count;
		packetData->dataSize += dataSize * count;
		dAppLog(LOG_DEBUG, "\t[%d][%x][%dbyte][x%d]", *index, msgId, packetData->dataSize, packetData->count);
	}

	if(logHeader->msgId == 1) return 0;
	logHeader->msgId = 1;

	if(time(NULL) > logHeader->dataSize + 1000)
	{
		logHeader->dataSize = time(NULL);
		if(logHeader->count==0){
			logHeader->msgId = 0;
			ResetData();
			return 0;
		}
		int i = 1;
		while(logHeader->count >= i)	
		{
			packetData = (PacketTrafficData*)shmDBLog[i];
			dAppLog(LOG_DEBUG, "[%d]DBLOG ADD DATA [0x%x][%dbyte][x%d] [LOG_COUNT %d]", i, packetData->msgId, packetData->dataSize, packetData->count,  logHeader->count);
			if(!AddData(packetData, sizeof(PacketTrafficData)))
			{
				dAppLog(LOG_CRI, "-----DB LOGING ERROR-----AddData fail. Data was too big");
				break;
			}
			memset(packetData, 0, sizeof(PacketTrafficData));
			i++;
		}

		m_dataQueue.packetCount = logHeader->count;
		dAppLog(LOG_DEBUG, " Send DBLOG [count %d]", logHeader->count);
		int nRet =  msgsnd(DBlog_qid, &m_dataQueue, m_dataQueue.dataSize + sizeof(short) + sizeof(short), IPC_NOWAIT);
		if(nRet != 0)
		{
			dAppLog(LOG_CRI, "-----DB LOGING ERROR-----msgsnd fail. [ret %d]", nRet);
		}
		else
		{
			m_dataQueue.dataSize = 0;
		}

		dAppLog(LOG_DEBUG, " ResetData DBLOG [count %d]", logHeader->count);
		ResetData();
	}

	logHeader->msgId = 0;
	return 0;
}

int AddData(void* pData , int dataSize)
{
	if(m_dataQueue.dataSize + dataSize > MAX_PACKET_TRAFFIC_DATA_SIZE) 
	{
		printf("\n\t%d+%d data queue is Full %d/%d", m_dataQueue.dataSize, dataSize, m_dataQueue.dataSize, MAX_PACKET_TRAFFIC_DATA_SIZE);
		return -1;
	}

	memcpy(m_dataQueue.pData+m_dataQueue.dataSize, pData, dataSize);
	m_dataQueue.dataSize += dataSize;

	return 0;
}

int ResetData()
{
	logHeader->count = 0;
	shmDBLogIndex.Reset();
	return 0;
}