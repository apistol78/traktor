#ifndef traktor_InplaceRef_H
#define traktor_InplaceRef_H

#include "Core/Heap/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{

/*! \brief In-place reference.
 * \ingroup Core
 */
template < typename Class >
class InplaceRef
{
public:
	InplaceRef(Class*& ref)
	:	m_ref(ref)
	{
	}

	void replace(Class* ptr)
	{
		Heap::exchangeRef((void**)&m_ref, (void*)ptr);
	}

	Class* ptr() const
	{
		return m_ref;
	}

	// \name Dereference operators
	// @{

	operator Class* () const
	{
		return m_ref;
	}

	Class* operator -> ()
	{
		return m_ref;
	}

	// @}

	// \name Assign operators
	// @{

	InplaceRef& operator = (Class* ptr)
	{
		replace(ptr);
		return *this;
	}

	template < typename ArgumentClass >
	InplaceRef& operator = (const InplaceRef< ArgumentClass >& ref)
	{
		replace(ref.ptr());
		return *this;
	}

	template < typename ArgumentClass >
	InplaceRef& operator = (const Ref< ArgumentClass >& ref)
	{
		replace(ref.ptr());
		return *this;
	}

	// @}

	// \name Cast operators
	// @{

	template < typename ReturnType >
	operator Ref< ReturnType > ()
	{
		return m_ref;
	}

	// @}

	// \name Compare operators
	// @{

	bool operator < (const InplaceRef& rh) const
	{
		return m_ref < rh.m_ref;
	}

	// @}

private:
	Class*& m_ref;
};

}

#endif	// traktor_InplaceRef_H
