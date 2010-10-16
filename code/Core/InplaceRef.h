#ifndef traktor_InplaceRef_H
#define traktor_InplaceRef_H

#include "Core/IRefCount.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Inplace reference container.
 * \ingroup Core
 *
 * Useful as wrapper for an existing
 * object pointer; such as elements in an array.
 */
template < typename ClassType >
class InplaceRef
{
public:
	typedef ClassType* pointer;
	typedef ClassType& reference;
	typedef ClassType*& pointer_reference;

	InplaceRef(pointer_reference ref)
	:	m_ref(ref)
	{
	}

	pointer ptr() const
	{
		return m_ref;
	}

	const pointer c_ptr() const
	{
		return m_ref;
	}
	
	reference operator * () const
	{
		return *m_ref;
	}

	pointer operator -> ()
	{
		return m_ref;
	}
	
	const pointer operator -> () const
	{
		return m_ref;
	}
	
	operator pointer() const
	{
		return m_ref;
	}
	
	InplaceRef& operator = (const InplaceRef& ref)
	{
		T_SAFE_ADDREF(ref.m_ref);
		T_SAFE_RELEASE(m_ref);
		m_ref = ref.m_ref;
		return *this;
	}
	
	InplaceRef& operator = (pointer ptr)
	{
		T_SAFE_ADDREF(ptr);
		T_SAFE_RELEASE(m_ref);
		m_ref = ptr;
		return *this;
	}
	
private:
	pointer_reference m_ref;
};

}

#endif	// traktor_InplaceRef_H
