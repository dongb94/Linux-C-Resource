#include "DBLogAgent.h"

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <sys/types.h>   
#include <sys/ipc.h>   
#include <sys/msg.h>

#include <ElgradoDefine.h>

extern int	errno;

CDBLogAgent::CDBLogAgent(void){}
CDBLogAgent::~CDBLogAgent(void){}

int	CDBLogAgent::RunSkeleton()
{
	pPacketTrafficMsgQueue		stMsgQ;
	memset(stMsgQ, 0, sizeof(*stMsgQ));

	int			nRet;

	nRet = m_WrapperDB.InitDatabase();
	if (nRet < 0)
	{
		FinishProgram();
	}

	m_bStop = 1;
	while(m_bStop) 
	{
		nRet = RecvMessage(stMsgQ);

		if(nRet > 0)
			SwitchMessage(stMsgQ);

	}

	//
	FinishProgram();

	return 1;
}


int	CDBLogAgent::InitIPC()
{
	int nRet = CSkeleton::InitIPC();
	LinuxMessageCore stMsgQ;

	if(nRet < 0)
		return nRet;
	//

	DBLog_qid = msgget(LINUX_MESSAGE_ID_DBLOG, 0666 | IPC_CREAT);
	if(DBLog_qid < 0)
	{
		dAppLog(LOG_CRI, "FAIL [m_msgDBLOG.SetIdentify]");
		return -1;
	}

	m_WrapperDB.m_nProcessID = GetProcessID();

	return 0;
}

int CDBLogAgent::RecvMessage(pPacketTrafficMsgQueue msgQueue)
{
	int res = 0;
	res = msgrcv(DBLog_qid, msgQueue, sizeof(*msgQueue) - sizeof(long int), 0, MSG_NOERROR);
	if(res < 0)
	{
		if(errno != EINTR && errno != ENOMSG)
		{
			dAppLog(LOG_DEBUG,"[FAIL:%d] MSGRCV MYQ : [%s]", errno, strerror(errno));
			return -1;	  /* Error */
		}

		return 0;   /* Do Nothing */
	}
	
	return res;
}

int CDBLogAgent::SwitchMessage(pPacketTrafficMsgQueue pstMsgQ)
{
	PacketTrafficData* data = (PacketTrafficData*)& pstMsgQ->pData[0];
	for(int i =0; i<pstMsgQ->packetCount; i++)
	{
		m_WrapperDB.DBLOG(data->msgId, data->count, data->dataSize);
		data+=sizeof(PacketTrafficData);
	}

	// dAppLog(LOG_DEBUG, " PRINT LOG [MsgId %x][count %d][dataSize %d]", data->msgId, data->count, data->dataSize);

	return 0;
}

