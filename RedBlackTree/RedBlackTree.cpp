#include "RedBlackTree.h"

char buffer[200];
void RedBlackTree::PrintTree(RBTreeNode *node, int depth)
{
	if(node == NULL) return;

	sprintf(buffer + depth, "(%d,%d)[%d]\n\0", node->value.Key, node->value.Value, node->color);
	std::cout<<buffer;

	sprintf(buffer+depth, "L");
	PrintTree(node->left, depth+1);
	sprintf(buffer+depth, "R");
	PrintTree(node->right, depth+1);
}

RedBlackTree::RedBlackTree(int maxSize, char* memStart)
{
	m_size = 0;
	root = NULL;

	m_maxSize = maxSize;
	m_memStart = memStart;

	memset((void *)memStart, 0, sizeof(RBTreeNode) * maxSize);
	
}
RedBlackTree::~RedBlackTree(){}

RBTreeNode* RedBlackTree::insert(unsigned long long key, unsigned long long value)
{
	//TODO Add key duplicate action

	if(m_maxSize <= m_size) return NULL;

	RBTreeNode *newNode = (RBTreeNode *)(m_memStart + m_size * sizeof(RBTreeNode));
	newNode->color = RED; // RED
	newNode->value.Key = key;
	newNode->value.Value = value;

	if(root == NULL)
	{
		root = newNode;
		newNode->color = BLACK; // BLACK
	}
	else
	{
		insert(newNode, root);
	}

	m_size++;

	// PrintTree(root);
	// std::cout<<"Insert : ["<<newNode->value.Key<<","<<newNode->value.Value<<"] ["<<(int)(newNode->color)<<"] Size["<<m_size<<"]"<<newNode<<"\n";
	return newNode;
}

void RedBlackTree::insert(RBTreeNode *insertNode, RBTreeNode *parentNode)
{
	if((unsigned long long) insertNode->value.Key < (unsigned long long) parentNode->value.Key)
		node = &(parentNode->left);
	else
		node = &(parentNode->right);

	if(*node == NULL)
	{
		(*node) = insertNode;
		insertNode->parent = parentNode;

		CheckTree(insertNode);
	}
	else
	{
		if((unsigned long long) insertNode->value.Key < (unsigned long long) (parentNode)->value.Key)
			insert(insertNode, *(node));
		else
			insert(insertNode, *(node));
	}
}

// 오류 있음
void RedBlackTree::Remove(unsigned long long key)
{
	RBTreeNode *removeTarget = RemoveNode(key);

	if(removeTarget != NULL)
	{
		// delete removeTarget;

		removeTarget = NULL;

		m_size--;
	}

	// PrintTree(root);
	// std::cout<<"Remove : ["<<key<<"] Size["<<m_size<<"]"<<"\n";
}

RBTreeNode* RedBlackTree::RemoveNode(unsigned long long key)
{
	RBTreeNode *findNode = find(key);
	if(findNode == NULL) return NULL; // 타겟 없음

	RBTreeNode *removeTarget;
	RBTreeNode *largestLeftChild;
	if(findNode->left == NULL) //왼쪽 자식이 없는 경우
	{
		removeTarget = findNode;
		if(removeTarget == root)
		{
			root = NULL;
			return removeTarget;
		}

		if(removeTarget->parent->left == removeTarget)
		{
			removeTarget->parent->left = removeTarget->right;
		}
		else
		{
			removeTarget->parent->right = removeTarget->right;
		}

		if(removeTarget->color == RED) // 삭제 노드가 RED면 추가 조치 없음.
		{
			return removeTarget;
		}

		if(removeTarget->right != NULL) // 왼쪽에 자식이 없으면 오른쪽 자식은 없거나 RED
		{
			removeTarget->right->parent = removeTarget->parent;
			removeTarget->right->color = BLACK;
			return removeTarget;
		}

		// 
		RemoveDoubleBlack(NULL, removeTarget->parent);
	}
	else // 왼쪽 자식이 있는 경우
	{
		largestLeftChild = findNode->left;
		while(largestLeftChild->right != NULL)
		{
			largestLeftChild = largestLeftChild->right;
		}

		removeTarget = largestLeftChild;
		
		memcpy(&(findNode->value), &(removeTarget->value), sizeof(NodeValue));

		// remove target 삭제
		if(removeTarget->parent->left==removeTarget) // 왼쪽에 자식이 1개밖에 없거나, BLACK LEFT 자식과 RED LEFT LEFT 손자만 있는 경우
		{
			removeTarget->parent->left = removeTarget->left;
		}
		else
		{
			removeTarget->parent->right = removeTarget->left;
		}

		if(removeTarget->left != NULL) // 오른쪽 자식이 없으므로 왼쪽 자식이 있다면 무조건 RED이다.
		{
			removeTarget->left->parent = removeTarget->parent;
			removeTarget->left->color = BLACK;
			return removeTarget;
		}

		if(removeTarget->color == BLACK) // 삭제 NODE가 RED면 추가 조치가 필요 없다.
		{
			RemoveDoubleBlack(NULL, removeTarget->parent);
		}
	}
	return removeTarget;
}

RBTreeNode* RedBlackTree::find(unsigned long long key)
{
	if(root == NULL) return NULL;

	RBTreeNode *pointer = root;
	while (pointer != NULL)
	{
		// std::cout<<"FindKey : "<<key<<" [C]["<<pointer->value.Key<<"] ["<<(int)(pointer->color)<<"] ["<<pointer->value.Value<<"]"<<pointer<<"\n";
		if(key == pointer->value.Key)
		{
			// std::cout<<"Find Node ["<<pointer->value.Key<<","<<pointer->value.Value<<"]"<<pointer<<"\n";
			return pointer;
		}
		else if(key < pointer->value.Key)
		{
			pointer = pointer->left;
		}
		else
		{
			pointer = pointer->right;
		}
	}
	return NULL;
}

int RedBlackTree::Lotate(RBTreeNode *item)
{
	if(item->parent->left == item)
	{
		RightLotate(item);
		return 1;
	}
	else if(item->parent->right == item)
	{
		LeftLotate(item);
		return 2;
	}
	else
	{
		return -1;
	}
}

void RedBlackTree::RightLotate(RBTreeNode *item)
{
	RBTreeNode *parent, *grandParent, *rightChild;
	parent = item->parent;
	grandParent = parent->parent;
	rightChild = item->right;

	if(grandParent != NULL)
	{
		if(grandParent->right == parent) // parent is right node
		{
			grandParent->right = item;
		}
		else
		{
			grandParent->left = item;
		}
	}

	item->parent = grandParent;
	item->right = parent;

	parent->parent = item;
	parent->left = rightChild;

	if(rightChild != NULL)
		rightChild->parent = parent;
}

void RedBlackTree::LeftLotate(RBTreeNode *item)
{
	RBTreeNode *parent, *grandParent, *leftChild;
	parent = item->parent;
	grandParent = parent->parent;
	leftChild = item->left;

	if(grandParent != NULL)
	{
		if(grandParent->right == parent) // parent is right node
		{
			grandParent->right = item;
		}
		else
		{
			grandParent->left = item;
		}
	}

	item->parent = grandParent;
	item->left = parent;

	parent->parent = item;
	parent->right = leftChild;

	if(leftChild != NULL)
		leftChild->parent = parent;
}

void RedBlackTree::Restructuring(RBTreeNode *item)
{
	RBTreeNode *parent, *grandParent;
	parent = item->parent;
	grandParent = parent->parent;

	if(grandParent == NULL) return;

	RBTreeNode *tem;

	if(parent->left == item) // node is left node
	{
		if(grandParent->right == parent) // parent is right node
		{
			RightLotate(item);
			LeftLotate(item);
			tem = parent;
			parent = item;
			item = tem;
		}
		else
		{
			// left - left rotate
			RightLotate(parent);
		}
	}
	else if(parent->right == item)// node is right node
	{
		if(grandParent->left == parent) // parent is left node
		{
			LeftLotate(item);
			RightLotate(item);
			tem = parent;
			parent = item;
			item = tem;
		}
		else
		{
			// right - right rotate
			LeftLotate(parent);
		}
	}
	else
	{
		// Critical Exception
	}

	// change color
	parent->color = BLACK;
	grandParent->color = RED; // red

	if(grandParent == root)
	{
		root = parent;
	}
	
}

void RedBlackTree::CheckTree(RBTreeNode *item)
{
	RBTreeNode *parentNode = item->parent;
	if(parentNode == NULL) // item == root
	{
		return;
	}
	if(item->color == RED && parentNode->color == RED) // 연속으로 Red가 2개 인 경우
	{
		RBTreeNode *uncle;
		if(parentNode == parentNode->parent->left)
		{
			uncle = parentNode->parent->right;
		}
		else if (parentNode == parentNode->parent->right)
		{
			uncle = parentNode->parent->left;
		}
		else
		{
			// Critical Exception
		}

		if(uncle == NULL || uncle->color == BLACK) // uncle 이 leaf node이거나 블랙 노드 일 경우
		{
			Restructuring(item);
		}
		else
		{
			// recoloring
			uncle->color = BLACK;
			parentNode->color = BLACK;

			if(parentNode->parent == root)
			{
				parentNode->parent->color = BLACK;
			}
			else
			{
				parentNode->parent->color = RED;
			}

			CheckTree(parentNode->parent);
		}
	}
}


void RedBlackTree::RemoveDoubleBlack(RBTreeNode *doubleBlack, RBTreeNode *parent)
{
	bool isLeft;
	RBTreeNode *sbling;
	if(parent->left == doubleBlack)
	{
		isLeft = true;
		sbling = parent->right;
	}
	else
	{
		isLeft = false;
		sbling = parent->left;
	}

	if(sbling->color == RED) // 형제가 RED면 부모는 BLACK
	{
		//case 0
		Lotate(sbling);
		sbling->color = BLACK;
		parent->color = RED;

		RemoveDoubleBlack(doubleBlack, parent);
	}
	else // 형제가 BLACK 인경우
	{
		char colorLeft, colorRight;
		if(sbling->left != NULL && sbling->left->color == RED) colorLeft = RED;
		if(sbling->right != NULL && sbling->right->color == RED) colorRight = RED;
		//case 1 double black
		if(colorLeft==BLACK && colorRight==BLACK)
		{
			sbling->color = RED;
			if(parent->color==RED)
			{
				parent->color=BLACK;
			}
			else
			{
				RemoveDoubleBlack(parent, parent->parent);
			}
		}
		else if(isLeft)
		{
			//case 2 is left and right nephew is red
			if(colorRight==RED)
			{
				RightLotate(sbling);
			}
			//case 3 is left but right nephew is black
			else
			{
				RBTreeNode *leftNephew = sbling->left;
				leftNephew->color = BLACK;
				LeftLotate(sbling->left);
				RightLotate(leftNephew);
			}
		}
		else
		{
			//case 4 is right and left nephew is red
			if(colorLeft==RED)
			{
				LeftLotate(sbling);
			}
			//case 5 is right but left nephew is black
			else
			{
				RBTreeNode *rightNephew = sbling->right;
				rightNephew->color = BLACK;
				RightLotate(sbling->right);
				LeftLotate(rightNephew);
			}
		}
	}
}