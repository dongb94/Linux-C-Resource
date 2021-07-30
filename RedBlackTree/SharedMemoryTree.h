#ifndef _SHAEDEFEM_EMORURUMOMORY_TREE_H
#define _SHAEDEFEM_EMORURUMOMORY_TREE_H

#ifndef MAKE_SHM_KEY_FROM_INDEX
#define MAKE_SHM_KEY_FROM_INDEX(index) (((unsigned long long)index)<<48)+1
#endif // !MAKE_SHM_KEY_FROM_INDEX

#include <Skeleton.h>
#include <hashshm.h>
#include <shmutil.h>

#include <ServerSocket.h>
#include <Packet.h>

#include <RedBlackTree.h>

class CSharedMemoryTree
{
public:
	CSharedMemoryTree();
	~CSharedMemoryTree();

	int   CreateArray(key_t KeyValue, unsigned int ArrayNum, unsigned int ArraySize);

	char* operator[] (int key);		// 키로 검색		O(logN)
	char* GetArray(int index);		// 인덱스로 검색	O(1)

	char* Add(UINT64 key);
	int Remove(UINT64 key);

	void	printTree();

	int	m_count;
	int	m_nSizeNum;

private:
	st_HashedShmHandle		m_hsm_Memory;
	RedBlackTree			m_rbTree;

	int  m_nArraySize;
	char *m_pData;
};

#endif
