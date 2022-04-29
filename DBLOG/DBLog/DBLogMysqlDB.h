#ifndef _MYSQL_WRAPPERERPERERE_DBLOG
#define _MYSQL_WRAPPERERPERERE_DBLOG

#include <Skeleton.h>
#include <hashshm.h>
#include <shmutil.h>

#include <GameStructer.h>
#include <ServerSocket.h>
#include <Packet.h>


#include <RecvMessage.h>
#include <SendMessage.h>

#include <mysql.h>
#include <SharedMemory.h>
#include <PacketCore.h>

class CDBLogMysqlDB
{
public:
	CDBLogMysqlDB();
	~CDBLogMysqlDB();

	int InitDatabase();
	int CloseDatabase();

	int		m_nProcessID;

	ServerType		ThisServerType;

	int DBLOG(USHORT msgId, USHORT count, UINT32 bytesLength);

private:
	//CPacketCore		m_PacketCore;
	MYSQL			m_DBConn;
	
	//
	int InitMemory();

	void FreeResult(MYSQL_RES* res);
	void mysql_err_log(MYSQL *conn);
	int  RunQuery(MYSQL_RES **result, char* szQueryText);

	//
	CSharedMemory		m_sm_MyServerInfo, m_sm_ServersInfo, m_sm_ServiceInfo;

	int		m_nMyServerKey;
	bool	m_isFirstSystemCheck;



};

#endif // #ifndef _MYSQL_WRAPPERERPERERE_DBLOG
