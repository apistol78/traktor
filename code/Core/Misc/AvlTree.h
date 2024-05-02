/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"

namespace traktor
{

/*! \ingroup Core */
//@{

template < typename KeyType >
struct DefaultCompare
{
	static int32_t compare(const KeyType& a, const KeyType& b)
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

		explicit Node(const KeyType& key, const ItemType& item)
		:	left(nullptr)
		,	right(nullptr)
		,	key(key)
		,	item(item)
		{
		}

		int32_t depth() const
		{
			const int32_t ld = left ? left->depth() + 1 : 0;
			const int32_t rd = right ? right->depth() + 1 : 0;
			return std::max< int32_t >(ld, rd);
		}
	};

public:
	struct iterator
	{
		Node* node;

		explicit iterator(Node* node_)
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

		bool operator == (const iterator& i) const
		{
			return bool(node == i.node);
		}

		bool operator != (const iterator& i) const
		{
			return bool(node != i.node);
		}
	};

	void insert(const KeyType& key, const ItemType& item)
	{
		m_root = m_root ? _insert(m_root, key, item) : new Node(key, item);
	}

	iterator find(const KeyType& key)
	{
		return iterator(_find(m_root, key));
	}

	void erase(iterator& iter)
	{
	}

	iterator end()
	{
		return iterator(nullptr);
	}

private:
	Node* m_root = nullptr;

	Node* _insert(Node* node, const KeyType& key, const ItemType& item) const
	{
		const int32_t cmp = KeyCompare::compare(key, node->key);
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

	Node* _balance(Node* node) const
	{
		const int32_t ld = node->left ? node->left->depth() : 0;
		const int32_t rd = node->right ? node->right->depth() : 0;

		if (abs(ld - rd) < 2)
			return node;

		if (ld < rd)
		{
			Node* r = node->right;

			Node* lc = r->left;
			Node* rc = r->right;

			const int32_t lcd = lc ? lc->depth() : 0;
			const int32_t rcd = rc ? rc->depth() : 0;

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

			const int32_t lcd = lc ? lc->depth() : 0;
			const int32_t rcd = rc ? rc->depth() : 0;

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

	Node* _find(Node* node, const KeyType& key) const
	{
		if (node)
		{
			const int32_t cmp = KeyCompare::compare(key, node->key);
			if (cmp < 0)
				return _find(node->left, key);
			else if (cmp > 0)
				return _find(node->right, key);
		}
		return node;
	}
};

//@}

}

