#ifndef traktor_RefBase_H
#define traktor_RefBase_H

#include "Core/Config.h"
#include "Core/Heap/Heap.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{

/*! \brief Reference interface.
 * \ingroup Core
 *
 * All reference containers must implement this interface
 * in order for the heap to be able to traverse live objects.
 */
class T_DLLCLASS RefBase
{
public:
	void* m_owner;
	RefBase* m_prev;
	RefBase* m_next;

	struct T_NOVTABLE IVisitor
	{
		virtual void operator () (void* object) = 0;
	};

	RefBase()
	:	m_owner(0)
	,	m_prev(0)
	,	m_next(0)
	{
	}
	
	virtual ~RefBase()
	{
	}

	virtual void visit(IVisitor& visitor) = 0;

	virtual void invalidate(void* object) = 0;
};

}

#endif	// traktor_RefBase_H
