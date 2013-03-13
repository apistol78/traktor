#ifndef traktor_ThreadsafeFifo_H
#define traktor_ThreadsafeFifo_H

#include "Core/Containers/AlignedVector.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Semaphore.h"

namespace traktor
{

/*! \brief Thread-safe FIFO queue.
 * \ingroup Core
 */
template < typename ItemType, typename LockType = Semaphore >
class ThreadsafeFifo
{
public:
	void put(const ItemType& item)
	{
		T_ANONYMOUS_VAR(Acquire< LockType >)(m_lock);
		m_items.push_back(item);
	}

	bool get(ItemType& outItem)
	{
		T_ANONYMOUS_VAR(Acquire< LockType >)(m_lock);
		if (!m_items.empty())
		{
			outItem = m_items.front();
			m_items.erase(m_items.begin());
			return true;
		}
		else
			return false;
	}

	void clear()
	{
		T_ANONYMOUS_VAR(Acquire< LockType >)(m_lock);
		m_items.clear();
	}

	bool empty() const
	{
		T_ANONYMOUS_VAR(Acquire< LockType >)(m_lock);
		return m_items.empty();
	}

	uint32_t size() const
	{
		T_ANONYMOUS_VAR(Acquire< LockType >)(m_lock);
		return m_items.size();
	}

private:
	mutable LockType m_lock; 
	AlignedVector< ItemType > m_items;
};

}

#endif	// traktor_ThreadsafeFifo_H
