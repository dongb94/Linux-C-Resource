#include "DBLogMysqlDB.h"

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include <time.h>
#include <sys/time.h>
#include <sys/types.h>   
#include <sys/ipc.h>   
#include <sys/msg.h>
#include <sys/stat.h>
#include <dirent.h>


#include <ElgradoDefine.h>


extern int	errno;


//
void CDBLogMysqlDB::FreeResult(MYSQL_RES* res)
{
	if (res != NULL)
	{
		mysql_free_result(res);
	}
}

void CDBLogMysqlDB::mysql_err_log(MYSQL *conn)
{
	dAppLog(LOG_CRI, "MYSQL][ERROR][NO:%u][%s", mysql_errno(conn), mysql_error(conn));
}

int CDBLogMysqlDB::RunQuery(MYSQL_RES **result, char* szQueryText)
{
	int dRet;
	int	num_row;

	//
	dRet = mysql_query(&m_DBConn, szQueryText);
	if (dRet)
		return -1;

	//
	*result = mysql_store_result(&m_DBConn);
	if (*result == NULL)
		return -1;

	num_row = mysql_num_rows(*result);

	return num_row;
}
////
CDBLogMysqlDB::CDBLogMysqlDB(){}
CDBLogMysqlDB::~CDBLogMysqlDB(){}


int CDBLogMysqlDB::InitMemory()
{
	int nRet = 0;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//m_sm_MyServerInfo, m_sm_ServersInfo, m_sm_ServiceInfo;
	if (m_sm_MyServerInfo.CreateArray(S_SSHM_MY_SERVER_INFO, 1, sizeof(MyServerInfo)) < 0)
		return -1;

	if (m_sm_ServersInfo.CreateArray(S_SSHM_SERVERS_INFO, MAX_SERVES_IN_ALMOND, sizeof(ServersInfo)) < 0)
		return -1;

	if (m_sm_ServiceInfo.CreateArray(S_SSHM_SERVICE_INFO, 1, sizeof(ServiceInfo)) < 0)
		return -1;

	
	return 0;
}

int CDBLogMysqlDB::InitDatabase()
{
	my_bool reconnect = true;

	mysql_init(&m_DBConn);
	mysql_options(&m_DBConn, MYSQL_OPT_RECONNECT, &reconnect);


	if (!mysql_real_connect(&m_DBConn, "monitor.almondgame.com", "root", "somr##", "AlmondServers", 35848, NULL, 0))
	{
		dAppLog(LOG_DEBUG, "FAIL: >> dConnectDB  CAN NOT CONNECT. [monitor.almondgame.com]   [%s", mysql_error(&m_DBConn));

		if (!mysql_real_connect(&m_DBConn, "192.168.0.12", "root", "somr##", "AlmondServers", 3306, NULL, 0))
		{
			dAppLog(LOG_DEBUG, "FAIL: >> dConnectDB  CAN NOT CONNECT. [192.168.0.12]   [%s", mysql_error(&m_DBConn));

			if (!mysql_real_connect(&m_DBConn, "127.0.0.1", "root", "somr##", "AlmondServers", 3306, NULL, 0))
			{
				dAppLog(LOG_DEBUG, "FAIL: >> dConnectDB  CAN NOT CONNECT. [127.0.0.1] [%s", mysql_error(&m_DBConn));
				return -1;
			}
		}
	}

	//
	mysql_query(&m_DBConn, "SET NAMES 'utf8'");
	mysql_query(&m_DBConn, "SET CHARACTER SET 'utf8'");
	mysql_query(&m_DBConn, "SET COLLATION_CONNECTION='utf8_unicode_ci'");

	//
	if (InitMemory() < 0)
	{
		return -1;
	}

	return 0;
}

int CDBLogMysqlDB::CloseDatabase()
{
	mysql_commit(&m_DBConn);
	mysql_close(&m_DBConn);

	return 0;
}

int CDBLogMysqlDB::DBLOG(USHORT MsgId, USHORT count, UINT32 bytesLength)
{
	char	szQueryText[2048];
	sprintf(szQueryText, "INSERT INTO `packetTraffic`(`MsgId`, `Count`, `Bytes`) VALUES (%x, %d, %d)"
						, MsgId, count, bytesLength);

	// dAppLog(LOG_DEBUG, "INSERT INTO `packetTraffic`(%x, %d, %d)" , MsgId, count, bytesLength);

	if (mysql_query(&m_DBConn, szQueryText))
	{
		dAppLog(LOG_CRI, "Err Update PacketTraffic [MsgId : %x][bytes : %d]", MsgId, bytesLength);
		return -1;
	}
}