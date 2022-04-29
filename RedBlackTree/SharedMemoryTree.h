/**
 * @file SharedMemoryTree.h
 * @author donggeon byeon (dongb94@gmail.com)
 * @brief 공유메모리 크기에 따른 키 값 제한을 해결하기 위해 레드 블렉 트리를 이용함
 * 키를 이용한 접근과 인덱스를 이용한 임의접근을 제공함
 * @version 1.0
 * @date 2022-04-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _SHAEDEFEM_EMORURUMOMORY_TREE_H
#define _SHAEDEFEM_EMORURUMOMORY_TREE_H

#ifndef MAKE_SHM_KEY_FROM_INDEX
#define MAKE_SHM_KEY_FROM_INDEX(index) (((unsigned long long)index)<<48)+1
#endif // !MAKE_SHM_KEY_FROM_INDEX

#include <lwrite.h>
#include <hashshm.h>
#include <shmutil.h>

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
	int Reset();

	void	printTree();

	int	*m_count;
	int	m_nSizeNum;

private:
	st_HashedShmHandle		m_hsm_Memory;
	RedBlackTree			m_rbTree;

	int  m_nArraySize;
	char *m_pData;
};

#endif
