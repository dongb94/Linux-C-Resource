#ifndef ___RED____BLACK_TREE_
#define ___RED____BLACK_TREE_

#include <iostream>
#include <cstring>
#include <stdio.h>
#include <errno.h>
#include <sys/shm.h> 
#include <sys/ipc.h> 

enum RedBlackNodeColor
{
	BLACK,
	RED
};

struct NodeValue
{
	unsigned long long Key;
	unsigned long long Value;
};

struct RBTreeNode
{
	int left, right, parent;
	NodeValue value;
	int index;
	char color;
};

struct RBHeader
{
	int			rootNode;
	int			size;
	int			maxSize;
};

// 자연수만을 Key로 받는 Red-Black Tree.
class RedBlackTree
{
private:
	
	char		*memStart;

	RBHeader	*m_header;
	RBTreeNode	*node;

	void PrintTree(RBTreeNode *root, int depth = 0);

	int Rotate(RBTreeNode *item);

	void RightRotate(RBTreeNode *item);
	void LeftRotate(RBTreeNode *item);

	void Restructuring(RBTreeNode *item);
	void CheckTree(RBTreeNode *item);

	void RemoveDoubleBlack(RBTreeNode *doubleBlack, RBTreeNode *parent);

	RBTreeNode* GetNode(int index);
	RBTreeNode* operator[](int index);

public:
	void PrintTree();
	RedBlackTree();
	~RedBlackTree();

	int	init(key_t key, int maxSize);
	int reset();

	RBTreeNode* insert(unsigned long long key, unsigned long long value);
	void insert(RBTreeNode *insertNode, RBTreeNode *parentNode);

	void Remove(unsigned long long key);
	RBTreeNode* RemoveNode(unsigned long long key);

	RBTreeNode* find(unsigned long long key);
};
#endif