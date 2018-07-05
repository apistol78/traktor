/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#pragma once

#include "Core/Class/AutoVerify.h"
#include "Core/Class/CastAny.h"
#include "Core/Class/IRuntimeDispatch.h"
#include "Core/Meta/MethodSignature.h"

namespace traktor
{

/*! \ingroup Core */
/*! \{ */

/*! \name Properties */
/*! \{ */

template <
	typename ClassType,
	typename ValueType
>
struct PropertySet : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(PropertySet);

	typedef typename MethodSignature< false, ClassType, void, ValueType >::method_t method_t;

	method_t m_method;

	PropertySet(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ValueType >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENT_COUNT(1)
		T_VERIFY_ARGUMENT_TYPE(0, ValueType)

		(mandatory_non_null_type_cast< ClassType* >(self)->*m_method)(
			CastAny< ValueType >::get(argv[0])
		);
		return Any();
	}
};

template <
	typename ClassType,
	typename ValueType,
	bool Const
>
struct PropertyGet : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(PropertyGet);

	typedef typename MethodSignature< Const, ClassType, ValueType >::method_t method_t;

	method_t m_method;

	PropertyGet(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ValueType >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENT_COUNT(0)

		ValueType value = (mandatory_non_null_type_cast< ClassType* >(self)->*m_method)();
		return CastAny< ValueType >::set(value);
	}
};

template <
	typename ClassType,
	typename ValueType
>
struct FnPropertySet : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(FnPropertySet);

	typedef void (*method_t)(ClassType*, ValueType);

	method_t m_method;

	FnPropertySet(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ValueType >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENT_COUNT(1)
		T_VERIFY_ARGUMENT_TYPE(0, ValueType)

		(*m_method)(
			mandatory_non_null_type_cast< ClassType* >(self),
			CastAny< ValueType >::get(argv[0])
		);
		return Any();
	}
};

template <
	typename ClassType,
	typename ValueType
>
struct FnPropertyGet : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(FnPropertyGet);

	typedef ValueType (*method_t)(ClassType*);

	method_t m_method;

	FnPropertyGet(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ValueType >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENT_COUNT(0)

		ValueType value = (*m_method)(mandatory_non_null_type_cast< ClassType* >(self));
		return CastAny< ValueType >::set(value);
	}
};

/*! \} */

/*! \} */

}
