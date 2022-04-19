
#include <stdio.h>
#include <stdlib.h>

#include "SharedMemoryTree.h"


CSharedMemoryTree::CSharedMemoryTree()
{
	m_count = 0;
	m_nSizeNum = 0;
	m_nArraySize = 0;
	m_pData = NULL;
}

CSharedMemoryTree::~CSharedMemoryTree(){}

int CSharedMemoryTree::CreateArray(key_t KeyValue, unsigned int ArrayNum, unsigned int ArraySize)
{
	Init_hashed_shm_handle(&m_hsm_Memory, ArrayNum, ArraySize, ArrayNum, KeyValue);

	int nRet = Init_hashed_shm(&m_hsm_Memory);
	if (nRet < 0)
	{
		dAppLog(LOG_CRI, "[CreateTree] [%lld] Init_hashed_shm: %s", KeyValue, Msg_hashed_shm(nRet));
		throw KeyValue;
		return -1;
	}

	m_nSizeNum = ArrayNum;
	m_nArraySize = ArraySize;

	nRet = m_rbTree.init(KeyValue + 1, ArrayNum, &m_count);
	if(nRet <0)
	{
		dAppLog(LOG_CRI, "[CreateTree] [%lld] init rbTree Error [Ret %d]", KeyValue, nRet);
		throw KeyValue;
		return -1;
	}

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
		// if(m_nArraySize == 2400)
		// dAppLog(LOG_DEBUG, "Add Key[%d]", key);
		return Add(key);
	}
	UINT64 shmKey = node->value.Value;
	// if(m_nArraySize == 2400)
	// dAppLog(LOG_DEBUG, "Find shmKey %d->%lld", key, shmKey);

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
		node = m_rbTree.insert(key, *m_count);
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

		memset(m_pData, 0, m_nArraySize);
	}

	// printf("ADD TREE [KEY %lld][NODE ( %lld , %lld )][Count %d]\n",key, node->value.Key, node->value.Value, m_count);

	return m_pData;
}

int CSharedMemoryTree::Remove(UINT64 key)
{
	if (key < 0)
		return -1;

	RBTreeNode *node = m_rbTree.find(key);
	if(node==NULL)
	{
		return 0;
	}

	if(*m_count <= 0)
	{
		dAppLog(LOG_DEBUG, "SharedMemoryTree Remove Error : m_count is 0 [key %llx]",key);
		return 0;
	}

	// printf("REMOVE TREE [KEY %lld][NODE ( %lld , %lld )][Count %d]\n",key, node->value.Key, node->value.Value, m_count-1);

	UINT64 shmKey = node->value.Value;

	int res;
	res = Get_hashed_shm(&m_hsm_Memory, MAKE_SHM_KEY_FROM_INDEX(shmKey), (void**)&m_pData);
	if(res < 0)
	{
		return 0;
	}

	char *updateData;
	res = Get_hashed_shm(&m_hsm_Memory, MAKE_SHM_KEY_FROM_INDEX((*m_count)-1), (void**)&updateData);
	if(res < 0)
	{
		dAppLog(LOG_CRI, "SharedMemoryTree Remove Error : Get Last node Error [count %d]",*m_count);
		return -1;
	}

	RBTreeNode *lastNode = m_rbTree.getLastNode();
	if(lastNode == NULL)
	{
		dAppLog(LOG_CRI, "SharedMemoryTree Remove Error : Last node IS NULL [count %d]",*m_count);
		return -1;
	}
	if(lastNode->value.Value != (*m_count)-1)
	{
		dAppLog(LOG_CRI, "SharedMemoryTree Remove Error : Last node info not currect [count %d][last node value %d]",*m_count, lastNode->value.Value);
		return 0;
	}

	// printf("\t1(%llx,%lld)<-(%llx,%lld)\n", node->value.Key, node->value.Value, lastNode->value.Key, lastNode->value.Value);
	memcpy(m_pData, updateData, m_nArraySize);
	lastNode->value.Value = shmKey;
	// printf("\t2(%llx,%lld)<-(%llx,%lld)\n", node->value.Key, node->value.Value, lastNode->value.Key, lastNode->value.Value);
	m_rbTree.Remove(key);
	// printf("\t3(%llx,%lld)<-(%llx,%lld)\n", node->value.Key, node->value.Value, lastNode->value.Key, lastNode->value.Value);

	return 0;
}

int CSharedMemoryTree::Reset()
{
	m_rbTree.reset();
	Get_hashed_shm(&m_hsm_Memory, MAKE_SHM_KEY_FROM_INDEX(0), (void**)&m_pData);
	memset(m_pData, 0, m_nArraySize * m_nSizeNum);
}

void CSharedMemoryTree::printTree()
{
	m_rbTree.PrintTree();
}