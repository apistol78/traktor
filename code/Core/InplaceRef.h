/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_InplaceRef_H
#define traktor_InplaceRef_H

#include "Core/IRefCount.h"
#include "Core/Meta/Traits.h"

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
	
	const InplaceRef& operator = (const InplaceRef& ref) const
	{
		T_SAFE_ADDREF(ref.m_ref);
		T_SAFE_RELEASE(m_ref);
		m_ref = ref.m_ref;
		return *this;
	}
	
	const InplaceRef& operator = (pointer ptr) const
	{
		T_SAFE_ADDREF(ptr);
		T_SAFE_RELEASE(m_ref);
		m_ref = ptr;
		return *this;
	}
	
private:
	pointer_reference m_ref;
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
typename IsPointer< T >::base_t* dynamic_type_cast(const InplaceRef< T0 >& obj)
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
typename IsPointer< T >::base_t* checked_type_cast(const InplaceRef< T0 >& obj)
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
typename IsPointer< T >::base_t* checked_type_cast(const InplaceRef< T0 >& obj)
{
	T_ASSERT (!(!AllowNull && obj.ptr() == 0));
	return checked_type_cast< typename IsPointer< T >::base_t* >(obj.ptr());
}

}

#endif	// traktor_InplaceRef_H
