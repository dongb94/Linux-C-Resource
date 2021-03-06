
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
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>


#include "SharedMemoryTree.h"


CSharedMemoryTree::CSharedMemoryTree()
{
	m_count = 0;
	m_nSizeNum = 0;
	m_nArraySize = 0;
	m_pData = NULL;
}

CSharedMemoryTree::~CSharedMemoryTree()
{}

int CSharedMemoryTree::CreateArray(key_t KeyValue, unsigned int ArrayNum, unsigned int ArraySize)
{
	Init_hashed_shm_handle(&m_hsm_Memory, ArrayNum, ArraySize, ArrayNum, KeyValue);

	int nRet = Init_hashed_shm(&m_hsm_Memory);
	if (nRet < 0)
	{
		dAppLog(LOG_CRI, "[CreateTree] [%x] Init_hashed_shm: %s", KeyValue, Msg_hashed_shm(nRet));
		return -1;
	}

	m_nSizeNum = ArrayNum;
	m_nArraySize = ArraySize;

	m_rbTree.init(KeyValue + 1, ArrayNum);

	return 1;
}

char* CSharedMemoryTree::operator[] (int key)
{
	if (key < 0)
	{
		dAppLog(LOG_CRI, "Get Tree Shm Error [index %d]", key);
		return NULL;
	}

	// m_rbTree.PrintTree();

	RBTreeNode *node = m_rbTree.find(key);
	if(node==NULL)
	{
		dAppLog(LOG_DEBUG, "Add Key[%d]", key);
		return Add(key);
	}
	UINT64 shmKey = node->value.Value;
	// dAppLog(LOG_DEBUG, "Find shmKey %lld", shmKey);

	int res;
	res = Get_hashed_shm(&m_hsm_Memory, MAKE_SHM_KEY_FROM_INDEX(shmKey), (void**)&m_pData);
	if(res < 0)
	{
		dAppLog(LOG_CRI, "GET Tree Shm Error [index %d]", shmKey);
		return NULL;
	}

	return m_pData;
}

char* CSharedMemoryTree::GetArray(int index)
{
	if (index < 0 || index >= m_nSizeNum)
	{
		dAppLog(LOG_CRI, "Get Tree Shm Error [MaxSize %d][index %d]", m_nSizeNum, index);
		return NULL;
	}

	int res;
	res = Get_hashed_shm(&m_hsm_Memory, MAKE_SHM_KEY_FROM_INDEX(index), (void**)&m_pData);
	if(res < 0)
	{
		res = New_hashed_shm(&m_hsm_Memory, MAKE_SHM_KEY_FROM_INDEX(index), (void**)&m_pData);
		if(res < 0)
		{
			dAppLog(LOG_CRI, "Make Tree Shm Error [index %d]", index);
			return NULL;
		}
	}

	return m_pData;
}

char* CSharedMemoryTree::Add(UINT64 key)
{
	if (key < 0)
		return NULL;

	RBTreeNode *node = m_rbTree.find(key);
	if(node==NULL)
	{
		node = m_rbTree.insert(key, m_count);
		if(node == NULL)
		{
			dAppLog(LOG_CRI, "Tree Is Full [InsertKey %d]", key);
			return NULL;
		}
	}
	UINT64 shmKey = node->value.Value;

	int res;
	res = Get_hashed_shm(&m_hsm_Memory, MAKE_SHM_KEY_FROM_INDEX(shmKey), (void**)&m_pData);
	if(res < 0)
	{
		res = New_hashed_shm(&m_hsm_Memory, MAKE_SHM_KEY_FROM_INDEX(shmKey), (void**)&m_pData);
		if(res < 0)
		{
			dAppLog(LOG_CRI, "Make Array Shm Tree Error [key %d]", shmKey);
			return NULL;
		}
	}

	m_count++;

	return m_pData;
}

int CSharedMemoryTree::Remove(UINT64 key)
{
	if (key < 0)
		return -1;

	if(m_count <= 0)
	{
		dAppLog(LOG_DEBUG, "SharedMemoryTree Remove Error : m_count is 0 [key %llx]",key);
		return 0;
	}

	RBTreeNode *node = m_rbTree.find(key);
	if(node==NULL)
	{
		return 0;
	}

	UINT64 shmKey = node->value.Value;

	int res;
	res = Get_hashed_shm(&m_hsm_Memory, MAKE_SHM_KEY_FROM_INDEX(shmKey), (void**)&m_pData);
	if(res < 0)
	{
		return 0;
	}

	char *updateData;
	res = Get_hashed_shm(&m_hsm_Memory, MAKE_SHM_KEY_FROM_INDEX(m_count-1), (void**)&updateData);
	if(res < 0)
	{
		dAppLog(LOG_CRI, "SharedMemoryTree Remove Error : Get Last node Error [count %d]",m_count);
		return -1;
	}

	RBTreeNode *lastNode = m_rbTree.getLastNode();
	if(lastNode == NULL)
	{
		dAppLog(LOG_CRI, "SharedMemoryTree Remove Error : Last node IS NULL [count %d]",m_count);
		return -1;
	}
	if(lastNode->value.Value != m_count-1)
	{
		dAppLog(LOG_CRI, "SharedMemoryTree Remove Error : Last node info not currect [count %d][last node value %d]",m_count, lastNode->value.Value);
		return 0;
	}

	memcpy(m_pData, updateData, m_nArraySize);
	lastNode->value.Value = shmKey;
	m_rbTree.Remove(key);

	m_count--;

	return 0;
}

void CSharedMemoryTree::printTree()
{
	m_rbTree.PrintTree();
}