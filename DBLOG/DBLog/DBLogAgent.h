#ifndef __DBLOGAGENTENRENTENTUHU__
#define __DBLOGAGENTENRENTENTUHU__

#include <Skeleton.h>
#include <hashshm.h>
#include <shmutil.h>

#include <ServerSocket.h>
#include <GameStructer.h>

#include "DBLogMysqlDB.h"
#include <WriteDBLog.h>

class CDBLogAgent : public CSkeleton
{
public:
	CDBLogAgent(void);
	virtual ~CDBLogAgent(void);
	
	virtual int	RunSkeleton();
	virtual int	InitIPC();
		
private:
	int		RecvMessage(pPacketTrafficMsgQueue pstMsgQ);
	int		SwitchMessage(pPacketTrafficMsgQueue pstMsgQ);

	int DBLog_qid;

	CDBLogMysqlDB	m_WrapperDB;

	time_t		m_currentTime;

};


#endif //ifndef __DBLOGAGENTENRENTENTUHU__

