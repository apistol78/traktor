#ifndef traktor_Ref_H
#define traktor_Ref_H

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

/*! \brief Single object reference container.
 * \ingroup Core
 */
template < typename Class > class Ref : public RefBase
{
public:
	Ref()
	:	m_ptr(0)
	{
		Heap::registerRef(this, 0);
	}

	Ref(Class* ptr)
	:	m_ptr(ptr)
	{
		Heap::registerRef(this, (void*)m_ptr);
	}

	Ref(const Ref& ref)
	:	m_ptr(ref.m_ptr)
	{
		Heap::registerRef(this, (void*)m_ptr);
	}

	template < typename ArgumentClass >
	Ref(const Ref< ArgumentClass >& ref)
	:	m_ptr(ref.ptr())
	{
		Heap::registerRef(this, (void*)m_ptr);
	}

	virtual ~Ref()
	{
		Heap::unregisterRef(this, (void*)m_ptr);
	}
	
	void replace(Class* ptr)
	{
		Heap::exchangeRef((void**)&m_ptr, (void*)ptr);
	}

	Class* ptr() const
	{
		return m_ptr;
	}

	// \name Dereference operators
	// @{

	operator Class* () const
	{
		return m_ptr;
	}

	Class* operator -> () const
	{
		T_FATAL_ASSERT_M(m_ptr != 0, L"Null pointer access");
		return m_ptr;
	}

	// @}

	// \name Assignment operator.
	// @{

	Ref& operator = (const Ref& ref)
	{
		replace(ref.m_ptr);
		return *this;
	}

	// @}

	// \name Compare operators
	// @{

	bool operator < (const Ref& rh) const
	{
		return m_ptr < rh.m_ptr;
	}

	// @}

protected:
	virtual void visit(IVisitor& visitor)
	{
		visitor((void*)m_ptr);
	}

	virtual void invalidate(void* object)
	{
		if (object == m_ptr)
			m_ptr = 0;
	}

private:
	Class* m_ptr;
};

}

#endif	// traktor_Ref_H
