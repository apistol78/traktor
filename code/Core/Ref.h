#ifndef traktor_Ref_H
#define traktor_Ref_H

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

/*! \brief Single object reference container.
 * \ingroup Core
 */
template < typename Class >
class Ref
{
public:
	typedef Class* value_type;
	typedef value_type pointer;

	Ref()
	:	m_ptr(0)
	{
	}

	Ref(pointer ptr)
	:	m_ptr(ptr)
	{
		T_SAFE_ADDREF(m_ptr);
	}

	Ref(const Ref& ref)
	:	m_ptr(ref.m_ptr)
	{
		T_SAFE_ADDREF(m_ptr);
	}

	template < typename ArgumentClass >
	Ref(const Ref< ArgumentClass >& ref)
	:	m_ptr(ref.ptr())
	{
		T_SAFE_ADDREF(m_ptr);
	}

	virtual ~Ref()
	{
		T_SAFE_RELEASE(m_ptr);
	}
	
	void reset()
	{
		T_SAFE_RELEASE(m_ptr);
		m_ptr = 0;
	}

	void replace(pointer ptr)
	{
		T_SAFE_ADDREF(ptr);
		T_SAFE_RELEASE(m_ptr);
		m_ptr = ptr;
	}

	pointer disown()
	{
		pointer ptr = m_ptr;
		m_ptr = 0;
		return ptr;
	}

	pointer ptr() const
	{
		return m_ptr;
	}

	// \name Dereference operators
	// @{

	pointer operator -> () const
	{
		return m_ptr;
	}

	// @}

	// \name Assignment operator.
	// @{

	//Ref& operator = (pointer ptr)
	//{
	//	replace(ptr);
	//	return *this;
	//}

	Ref& operator = (const Ref& ref)
	{
		replace(ref.m_ptr);
		return *this;
	}

	// @}

	// \name Cast operators.
	// @{

	operator pointer () const
	{
		return m_ptr;
	}

	// @}

	// \name Compare operators
	// @{

	bool operator < (const Ref& rh) const
	{
		return m_ptr < rh.m_ptr;
	}

	// @}

private:
	pointer m_ptr;
};

}

#endif	// traktor_Ref_H
