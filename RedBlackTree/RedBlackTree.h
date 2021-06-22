#ifndef ___RED____BLACK_TREE_
#define ___RED____BLACK_TREE_

#include <iostream>
#include <cstring>
#include <stdio.h>

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
	RBTreeNode *left, *right, *parent;
	NodeValue value;
	char color;
};

class RedBlackTree
{
private:

	int m_maxSize;
	char *m_memStart;

	int m_size;
	
	RBTreeNode *root;
	RBTreeNode **node;

	void PrintTree(RBTreeNode *root, int depth = 0);

	int Rotate(RBTreeNode *item);

	void RightRotate(RBTreeNode *item);
	void LeftRotate(RBTreeNode *item);

	void Restructuring(RBTreeNode *item);
	void CheckTree(RBTreeNode *item);

	void RemoveDoubleBlack(RBTreeNode *doubleBlack, RBTreeNode *parent);
public:
	void PrintTree();
	RedBlackTree();
	~RedBlackTree();
	
	int	init(int maxSize, char* memStart);
	int loadTree();
	int reset();

	RBTreeNode* insert(unsigned long long key, unsigned long long value);
	void insert(RBTreeNode *insertNode, RBTreeNode *parentNode);

	void Remove(unsigned long long key);
	RBTreeNode* RemoveNode(unsigned long long key);

	RBTreeNode* find(unsigned long long key);
};
#endif