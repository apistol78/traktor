/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_AvlTree_H
#define traktor_AvlTree_H

namespace traktor
{

/*! \ingroup Core */
//@{

template < typename KeyType >
struct DefaultCompare
{
	static int compare(const KeyType& a, const KeyType& b)
	{
		return (a < b) ? -1 : ((a > b) ? 1 : 0);
	}
};

template <
	typename ItemType,
	typename KeyType,
	typename KeyCompare = DefaultCompare< KeyType >
>
class AvlTree
{
	struct Node
	{
		Node* left;
		Node* right;
		KeyType key;
		ItemType item;
		
		Node(const KeyType& key, const ItemType& item)
		:	left(0)
		,	right(0)
		,	key(key)
		,	item(item)
		{
		}

		int depth()
		{
			int ld = left ? left->depth() + 1 : 0;
			int rd = right ? right->depth() + 1 : 0;
			return std::max< int >(ld, rd);
		}
	};
	
public:
	struct Iterator
	{
		Node* node;

		Iterator(Node* node_)
		:	node(node_)
		{
		}

		ItemType& operator * ()
		{
			return node->item;
		}

		ItemType* operator -> ()
		{
			return &node->item;
		}

		bool operator == (const Iterator& i) const
		{
			return bool(node == i.node);
		}

		bool operator != (const Iterator& i) const
		{
			return bool(node != i.node);
		}
	};

	AvlTree() :
		m_root(0)
	{
	}

	void insert(const KeyType& key, const ItemType& item)
	{
		m_root = m_root ? _insert(m_root, key, item) : new Node(key, item);
	}

	Iterator find(const KeyType& key)
	{
		return Iterator(_find(m_root, key));
	}

	void erase(Iterator& iter)
	{
	}

	Iterator end()
	{
		return Iterator(0);
	}
	
private:
	Node* m_root;

	Node* _insert(Node* node, const KeyType& key, const ItemType& item)
	{
		int cmp = KeyCompare::compare(key, node->key);
		if (cmp < 0)
		{
			if (node->left)
			{
				node->left = _insert(node->left, key, item);
				node = _balance(node);
			}
			else
				node->left = new Node(key, item);
		}
		else if (cmp > 0)
		{
			if (node->right)
			{
				node->right = _insert(node->right, key, item);
				node = _balance(node);
			}
			else
				node->right = new Node(key, item);
		}
		else
		{
			node->item = item;
		}
		return node;
	}
	
	Node* _balance(Node* node)
	{
		int ld = node->left ? node->left->depth() : 0;
		int rd = node->right ? node->right->depth() : 0;
		
		if (abs(ld - rd) < 2)
			return node;

		if (ld < rd)
		{
			Node* r = node->right;

			Node* lc = r->left;
			Node* rc = r->right;

			int lcd = lc ? lc->depth() : 0;
			int rcd = rc ? rc->depth() : 0;

			if (lcd < rcd)
			{
				node->right = lc;
				r->left = node;
				node = r;
			}
			else
			{
				node->right = lc->left;
				r->left = lc->right;
				lc->right = r;
				lc->left = node;
				node = lc;
			}
		}
		else if (ld > rd)
		{
			Node* l = node->left;

			Node* lc = l->left;
			Node* rc = l->right;

			int lcd = lc ? lc->depth() : 0;
			int rcd = rc ? rc->depth() : 0;

			if (lcd > rcd)
			{
				node->left = rc;
				l->right = node;
				node = l;
			}
			else
			{
				node->left = rc->right;
				l->right = rc->left;
				rc->left = l;
				rc->right = node;
				node = rc;
			}
		}

		return node;
	}

	Node* _find(Node* node, const KeyType& key)
	{
		if (node)
		{
			int cmp = KeyCompare::compare(key, node->key);
			if (cmp < 0)
				return _find(node->left, key);
			if (cmp > 0)
				return _find(node->right, key);
		}
		return node;
	}
};

//@}

}

#endif	// traktor_AvlTree_H
