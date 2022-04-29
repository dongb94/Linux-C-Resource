#include "DBLogAgent.h"
#include <ElgradoDefine.h>

CDBLogAgent	g_theDemon;

void UserControlledSignal(int sign)
{
	g_theDemon.m_bStop = 0;
	g_theDemon.m_nFinishFlag = sign;
}

int main()
{

	g_theDemon.InitSkeleton(PROCESS_ID_DBLOG, "DB_LOG", "/RPG_Server/LOG", UserControlledSignal);
	g_theDemon.RunSkeleton();

	return 0;
}


//svn co svn://10.10.3.144/src/tech_server/landserver/trunk/CommandCenter___CSMS/_include ./