#include "RedBlackTree.h"

#pragma region PRINT
char buffer[200];
void RedBlackTree::PrintTree(RBTreeNode *node, int depth)
{
	if(node == NULL) return;

	sprintf(buffer + depth, "\t[%d](%d,%d)[P %d][LC %d][RC %d][C %d]\n\0", node->index, node->value.Key, node->value.Value, node->parent, node->left, node->right, node->color);
	std::cout<<buffer;

	sprintf(buffer+depth, "L");
	PrintTree(GetNode(node->left), depth+1);
	sprintf(buffer+depth, "R");
	PrintTree(GetNode(node->right), depth+1);
	std::cout<<"------------------\n";
}

void RedBlackTree::PrintTree()
{
	printf("HEADER %d[%llx] -STARTMEM %llx\n",m_header->rootNode, m_header, memStart);
	PrintTree(GetNode(m_header->rootNode));
}
#pragma endregion

RedBlackTree::RedBlackTree(){}
RedBlackTree::~RedBlackTree(){}

int	RedBlackTree::init(key_t key, int maxSize)
{
	if(maxSize == 0) return -1;
	int shmId = shmget(key, maxSize * sizeof(RBTreeNode), 0666 | IPC_CREAT | IPC_EXCL);
	if(shmId < 0)
	{
		if (errno == EEXIST)
		{
			errno = 0;
			shmId = shmget(key, maxSize * sizeof(RBTreeNode), 0666 | IPC_CREAT);
			if(shmId < 0)
			{
				printf("Make Shm Error [key[%d]]\n", key);
				return -4;
			}
		}
		else
		{
			printf("Make Shm Error [key[%d]]\n", key);
			return -5;
		}
	}

	m_header = (RBHeader *)shmat(shmId, 0, 0);
	memStart = (char*)m_header + sizeof(RBTreeNode);

	if(m_header->maxSize == 0)
	{
		m_header->maxSize = maxSize;
		reset();
	}

	return 0;
}

int	RedBlackTree::reset()
{
	printf("Reset tree [%llx]\n", m_header);

	m_header->rootNode = -1;
	m_header->size = 0;

	memset((void *)memStart, 0, sizeof(RBTreeNode) * m_header->maxSize);
}

RBTreeNode* RedBlackTree::insert(unsigned long long key, unsigned long long value)
{
	//TODO Add key duplicate action

	if(key == 0) 
	{
		printf("insert 0 key to red black tree\n");
		return NULL;
	}

	if(m_header->maxSize <= m_header->size) return NULL;

	RBTreeNode *newNode = (RBTreeNode *)(memStart + m_header->size * sizeof(RBTreeNode));
	// printf("HEADER %llx -STARTMEM %llx -NEWMEM %llx\n",m_header, memStart, newNode);
	newNode->color = RED; // RED
	newNode->value.Key = key;
	newNode->value.Value = value;
	newNode->index = m_header->size;
	newNode->parent = newNode->left = newNode->right = -1;

	if(m_header->rootNode == -1)
	{
		m_header->rootNode = m_header->size;
		newNode->color = BLACK; // BLACK
	}
	else
	{
		insert(newNode, GetNode(m_header->rootNode));
	}

	m_header->size++;

	// PrintTree();
	// std::cout<<"Insert : ["<<newNode->value.Key<<","<<newNode->value.Value<<"] ["<<(int)(newNode->color)<<"] Size["<<m_header->size<<"]"<<newNode<<"\n";
	return newNode;
}

void RedBlackTree::insert(RBTreeNode *insertNode, RBTreeNode *parentNode)
{
	int *index;
	if((unsigned long long) insertNode->value.Key < (unsigned long long) parentNode->value.Key)
		index = &(parentNode->left);
	else
		index = &(parentNode->right);

	if(*index == -1)
	{
		*index = insertNode->index;
		insertNode->parent = parentNode->index;

		CheckTree(insertNode);
	}
	else
	{
		insert(insertNode, GetNode(*index));
	}
}

// remove 기점으로 메모리가 바뀐다.
void RedBlackTree::Remove(unsigned long long key)
{
	RBTreeNode *removeTarget = RemoveNode(key);

	if(removeTarget != NULL)
	{
		// delete removeTarget;

		m_header->size--;

		if(m_header->size == removeTarget->index)
		{
			memset(removeTarget, 0, sizeof(RBTreeNode));
		}
		else
		{
			int index = removeTarget->index; // 기존 index 저장

			RBTreeNode *updateTarget = GetNode(m_header->size);

			memcpy(removeTarget, updateTarget, sizeof(RBTreeNode));
			memset(updateTarget, 0, sizeof(RBTreeNode));

			if(removeTarget->parent != -1)
			{
				updateTarget = GetNode(removeTarget->parent);
				if(updateTarget->right == removeTarget->index)
				{
					updateTarget->right = index;
				}
				else if(updateTarget->left == removeTarget->index)
				{
					updateTarget->left = index;
				}
				else
				{
					printf("[CRITICAL ERROR] Remove Red-Black Tree : Parant node index not currect [P %d][PL %d][PR %d][Target %d]\n", removeTarget->parent, updateTarget->left, updateTarget->right, m_header->size);
				}
			}

			if(removeTarget->left != -1)
			{
				updateTarget = GetNode(removeTarget->left);
				updateTarget->parent = index;
			}

			if(removeTarget->right != -1)
			{
				updateTarget = GetNode(removeTarget->right);
				updateTarget->parent = index;
			}

			removeTarget->index = index; // index 갱신
		}
	}
	// PrintTree();
	// std::cout<<"Remove : ["<<key<<"] Size["<<m_header->size<<"]"<<"\n";
}

RBTreeNode* RedBlackTree::RemoveNode(unsigned long long key)
{
	RBTreeNode *findNode = find(key);
	if(findNode == NULL) return NULL; // 타겟 없음

	RBTreeNode *removeTarget;
	RBTreeNode *largestLeftChild;
	if(findNode->left == -1) //왼쪽 자식이 없는 경우
	{
		removeTarget = findNode;
		if(removeTarget->index == m_header->rootNode)
		{
			m_header->rootNode = removeTarget->right;
			return removeTarget;
		}

		RBTreeNode *parent = GetNode(removeTarget->parent);
		if(parent->left == removeTarget->index)
		{
			parent->left = removeTarget->right;
		}
		else
		{
			parent->right = removeTarget->right;
		}

		if(removeTarget->color == RED) // 삭제 노드가 RED면 추가 조치 없음.
		{
			return removeTarget;
		}

		// 왼쪽에 자식이 없으면 오른쪽 자식은 없거나 RED
		if(removeTarget->right != -1) // RED인 자식이 있으면 부모를 수정하고 BLACK으로 변경
		{
			RBTreeNode *right = GetNode(removeTarget->right);
			right->parent=removeTarget->parent;
			right->color = BLACK;
			return removeTarget;
		}

		// 더블 블랙 확인
		RemoveDoubleBlack(NULL, GetNode(removeTarget->parent));
	}
	else // 왼쪽 자식이 있는 경우
	{
		largestLeftChild = GetNode(findNode->left);
		while(largestLeftChild->right != -1)
		{
			largestLeftChild = GetNode(largestLeftChild->right);
		}

		removeTarget = largestLeftChild;
		
		memcpy(&(findNode->value), &(removeTarget->value), sizeof(NodeValue));

		// remove target 삭제

		RBTreeNode *parent = GetNode(removeTarget->parent);
		if(GetNode(parent->left)==removeTarget) // 왼쪽에 자식이 1개밖에 없거나, BLACK LEFT 자식과 RED LEFT LEFT 손자만 있는 경우
		{
			parent->left = removeTarget->left;
			if(parent->left != -1) // RED LEFT LEFT 손자가 있는 경우
			{
				RBTreeNode *redGrandChild = GetNode(parent->left);
				redGrandChild->parent = parent->index;
				redGrandChild->color = BLACK;

				return removeTarget;
			}
		}
		else
		{
			parent->right = removeTarget->left;
		}

		if(removeTarget->left != -1) // 오른쪽 자식이 없으므로 왼쪽 자식이 있다면 무조건 RED이다. 왼쪽 자식이 RED 이면 삭제 노드는 무조건 BLACK.
		{
			RBTreeNode *left = GetNode(removeTarget->left);
			left->parent=removeTarget->parent;
			left->color = BLACK;
			return removeTarget;
		}

		if(removeTarget->color == BLACK) // 삭제 NODE가 RED면 추가 조치가 필요 없다.
		{
			RemoveDoubleBlack(NULL, GetNode(removeTarget->parent));
		}
	}
	return removeTarget;
}



RBTreeNode* RedBlackTree::find(unsigned long long key)
{
	if(m_header->rootNode == -1) return NULL;

	RBTreeNode *pointer = GetNode(m_header->rootNode);
	while (pointer != NULL)
	{
		//std::cout<<"FindKey : "<<key<<" [C]["<<pointer->value.Key<<"] ["<<(int)(pointer->color)<<"] ["<<pointer->value.Value<<"]"<<pointer<<"\n";
		if(key == pointer->value.Key)
		{
			//std::cout<<"Find Node ["<<pointer->value.Key<<","<<pointer->value.Value<<"]"<<pointer<<"\n";
			return pointer;
		}
		else if(key < pointer->value.Key)
		{
			pointer = GetNode(pointer->left);
		}
		else
		{
			pointer = GetNode(pointer->right);
		}
	}
	return NULL;
}

RBTreeNode* RedBlackTree::getLastNode()
{
	if(m_header->size == 0) return NULL;
	return (RBTreeNode *)(memStart + (m_header->size - 1) * sizeof(RBTreeNode));
}

int RedBlackTree::Rotate(RBTreeNode *item)
{
	RBTreeNode *parent = GetNode(item->parent);
	if(GetNode(parent->left) == item)
	{
		RightRotate(item);
		return 1;
	}
	else if(GetNode(parent->right) == item)
	{
		LeftRotate(item);
		return 2;
	}
	else
	{
		return -1;
	}
}

void RedBlackTree::RightRotate(RBTreeNode *item)
{
	if(item == NULL)
	{
		printf("NULL Rotate");
		return;
	}
	if(item->parent == -1) 
	{
		printf("Wrong Rotate P[%d] L[%d] R[%d]", item->parent, item->left, item->right);
		return;
	}

	RBTreeNode *parent, *grandParent, *rightChild;
	parent = GetNode(item->parent);
	grandParent = GetNode(parent->parent);
	rightChild = GetNode(item->right);

	
	//Check Tree Integrity
	if(parent->index != item->parent || 
		(grandParent!=NULL && grandParent->index != parent->parent) ||
		parent->left != item->index ||
		(rightChild != NULL && (rightChild->parent != item->index ||
		item->right != rightChild->index)))
	{
		if(grandParent != NULL)
			printf("<< TREE Integrity Error >> [GP %d %d %d %d]\n", grandParent->index, grandParent->parent, grandParent->left, grandParent->right);
		printf("<< TREE Integrity Error >> [P %d %d %d %d]\n", parent->index, parent->parent, parent->left, parent->right);
		printf("<< TREE Integrity Error >> [I %d %d %d %d]\n", item->index, item->parent, item->left, item->right);
		if(rightChild != NULL)
			printf("<< TREE Integrity Error >> [RC %d %d %d %d]\n", rightChild->index, rightChild->parent, rightChild->left, rightChild->right);
	}

	if(GetNode(parent->right) == item) return;

	if(grandParent != NULL)
	{
		if(GetNode(grandParent->right) == parent) // parent is right node
		{
			grandParent->right = item->index;
		}
		else
		{
			grandParent->left = item->index;
		}
	}
	else
	{
		m_header->rootNode = item->index;
	}

	item->parent = grandParent==NULL?-1:grandParent->index;
	item->right = parent->index;

	parent->parent = item->index;
	parent->left = rightChild==NULL?-1:rightChild->index;

	if(rightChild != NULL)
		rightChild->parent = parent->index;
}

void RedBlackTree::LeftRotate(RBTreeNode *item)
{
	if(item == NULL)
	{
		printf("NULL Rotate");
		return;
	}
	if(item->parent == -1) 
	{
		printf("Wrong Rotate P[%d] L[%d] R[%d]", item->parent, item->left, item->right);
		return;
	}

	RBTreeNode *parent, *grandParent, *leftChild;
	parent = GetNode(item->parent);
	grandParent = GetNode(parent->parent);
	leftChild = GetNode(item->left);

	//Check Tree Integrity
	if(parent->index != item->parent ||
		(grandParent!=NULL && grandParent->index != parent->parent) || 
		parent->right != item->index || 
		(leftChild != NULL && (leftChild->parent != item->index || 
		item->left != leftChild->index)))
	{
		if(grandParent != NULL)
			printf("<< TREE Integrity Error >> [GP %d %d %d %d]\n", grandParent->index, grandParent->parent, grandParent->left, grandParent->right);
		printf("<< TREE Integrity Error >> [P %d %d %d %d]\n", parent->index, parent->parent, parent->left, parent->right);
		printf("<< TREE Integrity Error >> [I %d %d %d %d]\n", item->index, item->parent, item->left, item->right);
		if(leftChild != NULL)
			printf("<< TREE Integrity Error >> [LC %d %d %d %d]\n", leftChild->index, leftChild->parent, leftChild->left, leftChild->right);
	}

	if(GetNode(parent->left) == item) return;

	if(grandParent != NULL)
	{
		if( GetNode(grandParent->right) == parent) // parent is right node
		{
			grandParent->right = item->index;
		}
		else
		{
			grandParent->left = item->index;
		}
	}
	else
	{
		m_header->rootNode = item->index;
	}

	item->parent = grandParent==NULL?-1:grandParent->index;
	item->left = parent->index;

	parent->parent = item->index;
	parent->right = leftChild==NULL?-1:leftChild->index;

	if(leftChild != NULL)
		leftChild->parent = parent->index;
}

void RedBlackTree::Restructuring(RBTreeNode *item)
{
	RBTreeNode *parent, *grandParent;
	parent = GetNode(item->parent);
	grandParent = GetNode(parent->parent);

	if(grandParent == NULL) return;

	RBTreeNode *tem;

	if(parent->left == item->index) // node is left node
	{
		if(grandParent->right == parent->index) // parent is right node
		{
			RightRotate(item);
			LeftRotate(item);
			tem = parent;
			parent = item;
			item = tem;
		}
		else
		{
			// left - left rotate
			RightRotate(parent);
		}
	}
	else if(parent->right == item->index)// node is right node
	{
		if(grandParent->left == parent->index) // parent is left node
		{
			LeftRotate(item);
			RightRotate(item);
			tem = parent;
			parent = item;
			item = tem;
		}
		else
		{
			// right - right rotate
			LeftRotate(parent);
		}
	}
	else
	{
		// Critical Exception
	}

	// change color
	parent->color = BLACK;
	grandParent->color = RED; // red
	
}

void RedBlackTree::CheckTree(RBTreeNode *item)
{
	RBTreeNode *parentNode = GetNode(item->parent);
	if(parentNode == NULL) // item == m_header->rootNode
	{
		return;
	}
	if(item->color == RED && parentNode->color == RED) // 연속으로 Red가 2개 인 경우
	{
		RBTreeNode *grandParent, *uncle;
		grandParent = GetNode(parentNode->parent);
		if(parentNode->index == grandParent->left)
		{
			uncle = GetNode(grandParent->right);
		}
		else if (parentNode->index == grandParent->right)
		{
			uncle = GetNode(grandParent->left);
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

			if(grandParent->index == m_header->rootNode)
			{
				grandParent->color = BLACK;
			}
			else
			{
				grandParent->color = RED;
			}

			CheckTree(grandParent);
		}
	}
}


void RedBlackTree::RemoveDoubleBlack(RBTreeNode *doubleBlack, RBTreeNode *parent)
{
	bool isLeft;
	RBTreeNode *sbling;
	if(GetNode(parent->left) == doubleBlack)
	{
		isLeft = true;
		sbling = GetNode(parent->right);
	}
	else if(GetNode(parent->right) == doubleBlack)
	{
		isLeft = false;
		sbling = GetNode(parent->left);
	}
	else // 무효
	{
		return;
	}

	if(sbling->color == RED) // 형제가 RED면 부모는 BLACK
	{
		//case 0
		Rotate(sbling);
		sbling->color = BLACK;
		parent->color = RED;

		RemoveDoubleBlack(doubleBlack, parent);
	}
	else // 형제가 BLACK 인경우
	{
		char colorLeft, colorRight;
		if(sbling->left != -1 && GetNode(sbling->left)->color == RED) colorLeft = RED;
		if(sbling->right != -1 && GetNode(sbling->right)->color == RED) colorRight = RED;
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
				RemoveDoubleBlack(parent, GetNode(parent->parent));
			}
		}
		else if(isLeft)
		{
			//case 2 is left and right nephew is red
			if(colorRight==RED)
			{
				RightRotate(sbling);
			}
			//case 3 is left but right nephew is black
			else
			{
				RBTreeNode *leftNephew = GetNode(sbling->left);
				leftNephew->color = BLACK;
				LeftRotate(leftNephew);
				RightRotate(leftNephew);
			}
		}
		else
		{
			//case 4 is right and left nephew is red
			if(colorLeft==RED)
			{
				LeftRotate(sbling);
			}
			//case 5 is right but left nephew is black
			else
			{
				RBTreeNode *rightNephew = GetNode(sbling->right);
				rightNephew->color = BLACK;
				RightRotate(rightNephew);
				LeftRotate(rightNephew);
			}
		}
	}
}

RBTreeNode* RedBlackTree::GetNode(int index)
{
	if(index < 0) return NULL;
	return (RBTreeNode*)(memStart + index * sizeof(RBTreeNode));

}

RBTreeNode* RedBlackTree::operator[](int index)
{
	if(index < 0) return NULL;
	return (RBTreeNode*)(memStart + index * sizeof(RBTreeNode));
}