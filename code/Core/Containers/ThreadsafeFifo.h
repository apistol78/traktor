/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <atomic>

namespace traktor
{

/*!
 */
template < typename T >
class ThreadsafeFifo
{
public:
	ThreadsafeFifo()
	{
		Node* dummy = new Node(T());
		m_head.store(dummy);
		m_tail.store(dummy);
	}

	~ThreadsafeFifo()
	{
		while (Node* oldHead = m_head.load())
		{
			m_head.store(oldHead->next);
			delete oldHead;
		}
	}

	void put(const T& item)
	{
		Node* newNode = new Node(item);
		Node* oldTail = nullptr;

		while (true)
		{
			oldTail = m_tail.load();
			Node* oldNext = oldTail->next.load();

			if (oldTail == m_tail.load())  // still consistent?
			{
				if (oldNext == nullptr)
				{
					if (oldTail->next.compare_exchange_strong(oldNext, newNode))
						break;
				}
				else
					m_tail.compare_exchange_strong(oldTail, oldNext);
			}
		}
		m_tail.compare_exchange_strong(oldTail, newNode);
	}

	bool get(T& item)
	{
		Node* oldHead = nullptr;
		while (true)
		{
			oldHead = m_head.load();
			Node* oldTail = m_tail.load();
			Node* oldNext = oldHead->next.load();

			if (oldHead == m_head.load())	// still consistent?
			{
				if (oldHead == oldTail)
				{
					if (oldNext == nullptr)  // queue is empty
						return false;
					m_tail.compare_exchange_strong(oldTail, oldNext);
				}
				else
				{
					item = oldNext->data;
					if (m_head.compare_exchange_strong(oldHead, oldNext))
						break;
				}
			}
		}
		delete oldHead;
		return true;
	}

	void clear()
	{
		T dummy;
		while (get(dummy));
	}

private:
	struct Node
	{
		T data;
		std::atomic< Node* > next;

		Node(const T& data)
		:	data(data)
		,	next(nullptr)
		{
		}
	};

	std::atomic< Node* > m_head;
	std::atomic< Node* > m_tail;
};

}
