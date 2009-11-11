#ifndef traktor_RefSet_H
#define traktor_RefSet_H

#include <algorithm>
#include <set>
#include "Core/Heap/RefBase.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{

/*! \brief Object set reference container.
 * \ingroup Core
 */
template < typename Class >
class RefSet : public RefBase
{
public:
	typedef std::set< Class* > container_type;
	typedef typename container_type::size_type size_type;
	typedef typename container_type::key_type key_type;
	typedef typename container_type::iterator iterator;
	typedef typename container_type::const_iterator const_iterator;

	RefSet()
	{
		Heap::registerRef(this, 0);
	}

	RefSet(const RefSet< Class >& s)
	{
		Heap::registerRef(this, 0);
		m_set = s.m_set;
		std::for_each(m_set.begin(), m_set.end(), Heap::incrementRef);
	}

	virtual ~RefSet()
	{
		clear();
		Heap::unregisterRef(this, 0);
	}

	void clear()
	{
		std::for_each(m_set.begin(), m_set.end(), Heap::decrementRef);
		m_set.clear();
	}

	size_t count() const
	{
		return m_set.count();
	}

	bool empty() const
	{
		return m_set.empty();
	}

	iterator begin()
	{
		return m_set.begin();
	}

	iterator end()
	{
		return m_set.end();
	}

	const_iterator begin() const
	{
		return m_set.begin();
	}

	const_iterator end() const
	{
		return m_set.end();
	}

	void erase(iterator iter)
	{
		Heap::decrementRef(*iter);
		m_set.erase(iter);
	}

	void erase(iterator first, iterator last)
	{
		std::for_each(first, last, Heap::decrementRef);
		m_set.erase(first, last);
	}

	void erase(const key_type& val)
	{
		std::set< Class* >::iterator i = m_set.find(val);
		if (i != m_set.end())
		{
			Heap::decrementRef(*i);
			m_set.erase(i);
		}
	}

	iterator find(const key_type& val)
	{
		return m_set.find(val);
	}

	const_iterator find(const key_type& val) const
	{
		return m_set.find(val);
	}

	void insert(Class* const val)
	{
		Heap::incrementRef(val);
		m_set.insert(val);
	}

	template < typename InputIterator >
	void insert(InputIterator first, InputIterator last)
	{
		for (InputIterator i = first; i != last; ++i)
			insert(*i);
	}

	size_type size() const
	{
		return m_set.size();
	}

	void swap(RefSet< Class >& right)
	{
		m_set.swap(right.m_set);
	}

protected:
	virtual void visit(IVisitor& visitor)
	{
		for (iterator i = begin(); i != end(); ++i)
			visitor((void*)(*i));
	}

	virtual void invalidate(void* object)
	{
		for (iterator i = begin(); i != end(); ++i)
		{
			if (*i == object)
				*i = 0;
		}
	}

private:
	container_type m_set;
};

}

#endif	// traktor_RefSet_H
