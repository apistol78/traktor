#ifndef traktor_ThreadsafeFifo_H
#define traktor_ThreadsafeFifo_H

#include <list>
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/Acquire.h"

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
		Acquire< LockType > key(m_lock);
		m_items.push_back(item);
	}

	bool get(ItemType& outItem)
	{
		Acquire< LockType > key(m_lock);
		if (!m_items.empty())
		{
			outItem = m_items.front();
			m_items.pop_front();
			return true;
		}
		else
			return false;
	}

private:
	LockType m_lock; 
	std::list< ItemType > m_items;
};

}

#endif	// traktor_ThreadsafeFifo_H
