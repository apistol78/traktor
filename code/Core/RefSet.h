#ifndef traktor_RefSet_H
#define traktor_RefSet_H

#include <algorithm>
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Object set reference container.
 * \ingroup Core
 */
template < typename Class >
class RefSet
{
public:
	typedef std::set< Class* > set_t;
	typedef typename set_t::const_iterator const_iterator;

	virtual ~RefSet()
	{
		clear();
	}

	void clear()
	{
		for (set_t::iterator i = m_items.begin(); i != m_items.end(); ++i)
			T_SAFE_RELEASE(*i);
		m_items.clear();
	}

	void insert(Class* item)
	{
		T_ASSERT (item);
		std::pair< set_t::iterator, bool > ins = m_items.insert(item);
		if (ins.second)
			T_SAFE_ADDREF(item);
	}

	const_iterator find(const Class* item) const
	{
		return m_items.find(item);
	}

	const_iterator begin() const
	{
		return m_items.begin();
	}

	const_iterator end() const
	{
		return m_items.end();
	}

private:
	set_t m_items;
};

}

#endif	// traktor_RefSet_H
