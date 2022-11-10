/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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

/*! Object reference container.
 * \ingroup Core
 */
template < typename ClassType >
class Ref
{
public:
	typedef ClassType* value_type;
	typedef ClassType* pointer;
	typedef ClassType& reference;

	/*! Empty constructor. */
	Ref()
	:	m_ptr(nullptr)
	{
	}

	/*! New reference to object. */
	Ref(const Ref& ref)
	:	m_ptr(ref.m_ptr)
	{
		T_SAFE_ADDREF(m_ptr);
	}

	/*! Move reference to object. */
	Ref(Ref&& ref) T_NOEXCEPT
	:	m_ptr(ref.m_ptr)
	{
		ref.m_ptr = nullptr;
	}

	/*! New naked reference to object. */
	Ref(pointer ptr)
	:	m_ptr(ptr)
	{
		T_SAFE_ADDREF(m_ptr);
	}

	/*! New reference to object including safe, implicit, down cast. */
	template < typename RefClassType >
	Ref(const Ref< RefClassType >& ref)
	:	m_ptr(static_cast< pointer >(ref))
	{
		T_SAFE_ADDREF(m_ptr);
	}

	/*! New reference to object including safe, implicit, down cast from inplace container. */
	template < typename InplaceClassType >
	Ref(const InplaceRef< InplaceClassType >& ref)
	:	m_ptr(static_cast< pointer >(ref))
	{
		T_SAFE_ADDREF(m_ptr);
	}

	/*! Release reference to object. */
	virtual ~Ref()
	{
		T_SAFE_RELEASE(m_ptr);
	}

	/*! Reset reference to object. */
	void reset()
	{
		T_SAFE_RELEASE(m_ptr);
		m_ptr = nullptr;
	}

	/*! Replace object which is referenced. */
	void replace(pointer ptr)
	{
		if (ptr != m_ptr)
		{
			T_SAFE_ADDREF(ptr);
			T_SAFE_RELEASE(m_ptr);
			m_ptr = ptr;
		}
	}

	/*! Get naked pointer to object. */
	pointer ptr() const
	{
		return m_ptr;
	}

	/*! Get naked, constant, pointer to object. */
	const pointer c_ptr() const
	{
		return m_ptr;
	}

	// \name Dereference operators
	// @{

	/*! Get reference to object. */
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

	Ref& operator = (Ref&& ref) T_NOEXCEPT
	{
		T_SAFE_RELEASE(m_ptr);
		m_ptr = ref.m_ptr;
		ref.m_ptr = nullptr;
		return *this;
	}

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

	/*! Compare pointer by value, used for sorted sets etc. */
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

/*! Dynamic cast object.
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

/*! Safe cast object.
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

/*! Safe cast object.
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
	T_ASSERT(!(!AllowNull && obj.ptr() == nullptr));
	return checked_type_cast< typename IsPointer< T >::base_t* >(obj.ptr());
}

/*! Safe cast object.
 *
 * The cast will cause system error if object is null or
 * of incorrect type.
 *
 * \param T Cast to type.
 * \param obj Object
 * \return Casted object.
 */
template < typename T, typename T0 >
typename IsPointer< T >::base_t* mandatory_non_null_type_cast(const Ref< T0 >& obj)
{
	T_FATAL_ASSERT (obj);
	return checked_type_cast< typename IsPointer< T >::base_t* >(obj.ptr());
}

}

