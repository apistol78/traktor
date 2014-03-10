#ifndef traktor_Ref_H
#define traktor_Ref_H

#include "Core/InplaceRef.h"

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
template < typename ClassType >
class Ref
{
public:
	typedef ClassType* value_type;
	typedef ClassType* pointer;
	typedef ClassType& reference;

	Ref()
	:	m_ptr(0)
	{
	}
	
	Ref(const Ref& ref)
	:	m_ptr(ref.m_ptr)
	{
		T_SAFE_ADDREF(m_ptr);
	}

#if defined(T_CXX11)
	Ref(Ref&& ref)
	:	m_ptr(ref.m_ptr)
	{
		ref.m_ptr = 0;
	}
#endif

	Ref(pointer ptr)
	:	m_ptr(ptr)
	{
		T_SAFE_ADDREF(m_ptr);
	}

	template < typename RefClassType >
	Ref(const Ref< RefClassType >& ref)
	:	m_ptr(static_cast< pointer >(ref))
	{
		T_SAFE_ADDREF(m_ptr);
	}

	template < typename InplaceClassType >
	Ref(const InplaceRef< InplaceClassType >& ref)
	:	m_ptr(static_cast< pointer >(ref))
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
	
	const pointer c_ptr() const
	{
		return m_ptr;
	}

	// \name Dereference operators
	// @{

	reference operator * () const
	{
		return *m_ptr;
	}
	
	pointer operator -> ()
	{
		return m_ptr;
	}
	
	const pointer operator -> () const
	{
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

#if defined(T_CXX11)
	Ref& operator = (Ref&& ref)
	{
		T_SAFE_RELEASE(m_ptr);
		m_ptr = ref.m_ptr;
		ref.m_ptr = 0;
		return *this;
	}
#endif

	Ref& operator = (pointer ptr)
	{
		replace(ptr);
		return *this;
	}

	template < typename RefClassType >
	Ref& operator = (const Ref< RefClassType >& ref)
	{
		replace(static_cast< pointer >(ref));
		return *this;
	}
	
	template < typename InplaceClassType >
	Ref& operator = (const InplaceRef< InplaceClassType >& ref)
	{
		replace(static_cast< pointer >(ref));
		return *this;
	}

	// @}

	// \name Cast operators.
	// @{

	operator pointer ()
	{
		return m_ptr;
	}
	
	operator const pointer () const
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

template < typename T, typename T0 >
T dynamic_type_cast(T0* obj);

template < typename T, typename T0 >
T dynamic_type_cast(const T0* obj);

template < typename T, typename T0 >
T checked_type_cast(T0* obj);

template < typename T, typename T0 >
T checked_type_cast(const T0* obj);

/*! \brief Dynamic cast object.
 *
 * \param T Cast to type.
 * \param o Object.
 * \return Casted value, null if object isn't of correct type.
 */
template < typename T, typename T0 >
typename IsPointer< T >::base_t* dynamic_type_cast(const Ref< T0 >& obj)
{
	return dynamic_type_cast< typename IsPointer< T >::base_t* >(obj.ptr());
}

/*! \brief Safe cast object.
 *
 * The cast will assert if object is of incorrect type.
 *
 * \param T Cast to type.
 * \param o Object.
 * \return Casted value.
 */
template < typename T, typename T0 >
typename IsPointer< T >::base_t* checked_type_cast(const Ref< T0 >& obj)
{
	return checked_type_cast< typename IsPointer< T >::base_t* >(obj.ptr());
}

/*! \brief Safe cast object.
 *
 * The cast will assert if object is of incorrect type.
 *
 * \param T Cast to type.
 * \param o Object.
 * \return Casted value.
 */
template < typename T, bool AllowNull, typename T0 >
typename IsPointer< T >::base_t* checked_type_cast(const Ref< T0 >& obj)
{
	T_ASSERT (!(!AllowNull && obj.ptr() == 0));
	return checked_type_cast< typename IsPointer< T >::base_t* >(obj.ptr());
}

}

#endif	// traktor_Ref_H
