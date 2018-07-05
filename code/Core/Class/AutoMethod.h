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

/*! \name Method invocations */
/*! \{ */

template <
	typename ClassType,
	typename ReturnType,
	bool Const
>
struct Method_0 : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(Method_0);

	typedef typename MethodSignature< Const, ClassType, ReturnType >::method_t method_t;

	method_t m_method;

	Method_0(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		ReturnType returnValue = (mandatory_non_null_type_cast< ClassType* >(self)->*m_method)();
		return CastAny< ReturnType >::set(returnValue);
	}
};

template <
	typename ClassType,
	bool Const
>
struct Method_0 < ClassType, void, Const > : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(Method_0);

	typedef typename MethodSignature< Const, ClassType, void >::method_t method_t;

	method_t m_method;

	Method_0(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void";
	}

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		(mandatory_non_null_type_cast< ClassType* >(self)->*m_method)();
		return Any();
	}
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	bool Const
>
struct Method_1 : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(Method_1);

	typedef typename MethodSignature< Const, ClassType, ReturnType, Argument1Type >::method_t method_t;

	method_t m_method;

	Method_1(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss); ss << L","; CastAny< Argument1Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENT_COUNT(1)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)

		ReturnType returnValue = (mandatory_non_null_type_cast< ClassType* >(self)->*m_method)(
			CastAny< Argument1Type >::get(argv[0])
		);
		return CastAny< ReturnType >::set(returnValue);
	}
};

template <
	typename ClassType,
	typename Argument1Type,
	bool Const
>
struct Method_1 < ClassType, void, Argument1Type, Const > : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(Method_1);

	typedef typename MethodSignature< Const, ClassType, void, Argument1Type >::method_t method_t;

	method_t m_method;

	Method_1(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void,"; CastAny< Argument1Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENT_COUNT(1)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)

		(mandatory_non_null_type_cast< ClassType* >(self)->*m_method)(
			CastAny< Argument1Type >::get(argv[0])
		);
		return Any();
	}
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	bool Const
>
struct Method_2 : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(Method_2);

	typedef typename MethodSignature< Const, ClassType, ReturnType, Argument1Type, Argument2Type >::method_t method_t;

	method_t m_method;

	Method_2(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss); ss << L","; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENT_COUNT(2)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)
		T_VERIFY_ARGUMENT_TYPE(1, Argument2Type)

		ReturnType returnValue = (mandatory_non_null_type_cast< ClassType* >(self)->*m_method)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1])
		);
		return CastAny< ReturnType >::set(returnValue);
	}
};

template <
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type,
	bool Const
>
struct Method_2 < ClassType, void, Argument1Type, Argument2Type, Const > : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(Method_2);

	typedef typename MethodSignature< Const, ClassType, void, Argument1Type, Argument2Type >::method_t method_t;

	method_t m_method;

	Method_2(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void,"; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENT_COUNT(2)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)
		T_VERIFY_ARGUMENT_TYPE(1, Argument2Type)

		(mandatory_non_null_type_cast< ClassType* >(self)->*m_method)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1])
		);
		return Any();
	}
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	bool Const
>
struct Method_3 : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(Method_3);

	typedef typename MethodSignature< Const, ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type >::method_t method_t;

	method_t m_method;

	Method_3(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss); ss << L","; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENT_COUNT(3)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)
		T_VERIFY_ARGUMENT_TYPE(1, Argument2Type)
		T_VERIFY_ARGUMENT_TYPE(2, Argument3Type)

		ReturnType returnValue = (mandatory_non_null_type_cast< ClassType* >(self)->*m_method)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2])
		);
		return CastAny< ReturnType >::set(returnValue);
	}
};

template <
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	bool Const
>
struct Method_3 < ClassType, void, Argument1Type, Argument2Type, Argument3Type, Const > : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(Method_3);

	typedef typename MethodSignature< Const, ClassType, void, Argument1Type, Argument2Type, Argument3Type >::method_t method_t;

	method_t m_method;

	Method_3(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void,"; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENT_COUNT(3)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)
		T_VERIFY_ARGUMENT_TYPE(1, Argument2Type)
		T_VERIFY_ARGUMENT_TYPE(2, Argument3Type)

		(mandatory_non_null_type_cast< ClassType* >(self)->*m_method)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2])
		);
		return Any();
	}
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	bool Const
>
struct Method_4 : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(Method_4);

	typedef typename MethodSignature< Const, ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type >::method_t method_t;

	method_t m_method;

	Method_4(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss); ss << L","; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENT_COUNT(4)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)
		T_VERIFY_ARGUMENT_TYPE(1, Argument2Type)
		T_VERIFY_ARGUMENT_TYPE(2, Argument3Type)
		T_VERIFY_ARGUMENT_TYPE(3, Argument4Type)

		ReturnType returnValue = (mandatory_non_null_type_cast< ClassType* >(self)->*m_method)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2]),
			CastAny< Argument4Type >::get(argv[3])
		);
		return CastAny< ReturnType >::set(returnValue);
	}
};

template <
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	bool Const
>
struct Method_4 < ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Const > : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(Method_4);

	typedef typename MethodSignature< Const, ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type >::method_t method_t;

	method_t m_method;

	Method_4(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void,"; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENT_COUNT(4)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)
		T_VERIFY_ARGUMENT_TYPE(1, Argument2Type)
		T_VERIFY_ARGUMENT_TYPE(2, Argument3Type)
		T_VERIFY_ARGUMENT_TYPE(3, Argument4Type)

		(mandatory_non_null_type_cast< ClassType* >(self)->*m_method)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2]),
			CastAny< Argument4Type >::get(argv[3])
		);
		return Any();
	}
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type,
	bool Const
>
struct Method_5 : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(Method_5);

	typedef typename MethodSignature< Const, ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type >::method_t method_t;

	method_t m_method;

	Method_5(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss); ss << L","; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss); ss << L","; CastAny< Argument5Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENT_COUNT(5)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)
		T_VERIFY_ARGUMENT_TYPE(1, Argument2Type)
		T_VERIFY_ARGUMENT_TYPE(2, Argument3Type)
		T_VERIFY_ARGUMENT_TYPE(3, Argument4Type)
		T_VERIFY_ARGUMENT_TYPE(4, Argument5Type)

		ReturnType returnValue = (mandatory_non_null_type_cast< ClassType* >(self)->*m_method)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2]),
			CastAny< Argument4Type >::get(argv[3]),
			CastAny< Argument5Type >::get(argv[4])
		);
		return CastAny< ReturnType >::set(returnValue);
	}
};

template <
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type,
	bool Const
>
struct Method_5 < ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Const > : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(Method_5);

	typedef typename MethodSignature< Const, ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type >::method_t method_t;

	method_t m_method;

	Method_5(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void,"; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss); ss << L","; CastAny< Argument5Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENT_COUNT(5)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)
		T_VERIFY_ARGUMENT_TYPE(1, Argument2Type)
		T_VERIFY_ARGUMENT_TYPE(2, Argument3Type)
		T_VERIFY_ARGUMENT_TYPE(3, Argument4Type)
		T_VERIFY_ARGUMENT_TYPE(4, Argument5Type)

		(mandatory_non_null_type_cast< ClassType* >(self)->*m_method)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2]),
			CastAny< Argument4Type >::get(argv[3]),
			CastAny< Argument5Type >::get(argv[4])
		);
		return Any();
	}
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type,
	typename Argument6Type,
	bool Const
>
struct Method_6 : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(Method_6);

	typedef typename MethodSignature< Const, ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type >::method_t method_t;

	method_t m_method;

	Method_6(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss); ss << L","; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss); ss << L","; CastAny< Argument5Type >::typeName(ss); ss << L","; CastAny< Argument6Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENT_COUNT(6)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)
		T_VERIFY_ARGUMENT_TYPE(1, Argument2Type)
		T_VERIFY_ARGUMENT_TYPE(2, Argument3Type)
		T_VERIFY_ARGUMENT_TYPE(3, Argument4Type)
		T_VERIFY_ARGUMENT_TYPE(4, Argument5Type)
		T_VERIFY_ARGUMENT_TYPE(5, Argument6Type)

		ReturnType returnValue = (mandatory_non_null_type_cast< ClassType* >(self)->*m_method)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2]),
			CastAny< Argument4Type >::get(argv[3]),
			CastAny< Argument5Type >::get(argv[4]),
			CastAny< Argument6Type >::get(argv[5])
		);
		return CastAny< ReturnType >::set(returnValue);
	}
};

template <
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type,
	typename Argument6Type,
	bool Const
>
struct Method_6 < ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Const > : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(Method_6);

	typedef typename MethodSignature< Const, ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type >::method_t method_t;

	method_t m_method;

	Method_6(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void,"; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss); ss << L","; CastAny< Argument5Type >::typeName(ss); ss << L","; CastAny< Argument6Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENT_COUNT(6)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)
		T_VERIFY_ARGUMENT_TYPE(1, Argument2Type)
		T_VERIFY_ARGUMENT_TYPE(2, Argument3Type)
		T_VERIFY_ARGUMENT_TYPE(3, Argument4Type)
		T_VERIFY_ARGUMENT_TYPE(4, Argument5Type)
		T_VERIFY_ARGUMENT_TYPE(5, Argument6Type)

		(mandatory_non_null_type_cast< ClassType* >(self)->*m_method)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2]),
			CastAny< Argument4Type >::get(argv[3]),
			CastAny< Argument5Type >::get(argv[4]),
			CastAny< Argument6Type >::get(argv[5])
		);
		return Any();
	}
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type,
	typename Argument6Type,
	typename Argument7Type,
	bool Const
>
struct Method_7 : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(Method_7);

	typedef typename MethodSignature< Const, ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type >::method_t method_t;

	method_t m_method;

	Method_7(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss); ss << L","; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss); ss << L","; CastAny< Argument5Type >::typeName(ss); ss << L","; CastAny< Argument6Type >::typeName(ss); ss << L","; CastAny< Argument7Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENT_COUNT(7)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)
		T_VERIFY_ARGUMENT_TYPE(1, Argument2Type)
		T_VERIFY_ARGUMENT_TYPE(2, Argument3Type)
		T_VERIFY_ARGUMENT_TYPE(3, Argument4Type)
		T_VERIFY_ARGUMENT_TYPE(4, Argument5Type)
		T_VERIFY_ARGUMENT_TYPE(5, Argument6Type)
		T_VERIFY_ARGUMENT_TYPE(6, Argument7Type)

		ReturnType returnValue = (mandatory_non_null_type_cast< ClassType* >(self)->*m_method)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2]),
			CastAny< Argument4Type >::get(argv[3]),
			CastAny< Argument5Type >::get(argv[4]),
			CastAny< Argument6Type >::get(argv[5]),
			CastAny< Argument7Type >::get(argv[6])
		);
		return CastAny< ReturnType >::set(returnValue);
	}
};

template <
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type,
	typename Argument6Type,
	typename Argument7Type,
	bool Const
>
struct Method_7 < ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Const > : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(Method_7);

	typedef typename MethodSignature< Const, ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type >::method_t method_t;

	method_t m_method;

	Method_7(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void,"; ss << CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss); ss << L","; CastAny< Argument5Type >::typeName(ss); ss << L","; CastAny< Argument6Type >::typeName(ss); ss << L","; CastAny< Argument7Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENT_COUNT(7)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)
		T_VERIFY_ARGUMENT_TYPE(1, Argument2Type)
		T_VERIFY_ARGUMENT_TYPE(2, Argument3Type)
		T_VERIFY_ARGUMENT_TYPE(3, Argument4Type)
		T_VERIFY_ARGUMENT_TYPE(4, Argument5Type)
		T_VERIFY_ARGUMENT_TYPE(5, Argument6Type)
		T_VERIFY_ARGUMENT_TYPE(6, Argument7Type)

		(mandatory_non_null_type_cast< ClassType* >(self)->*m_method)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2]),
			CastAny< Argument4Type >::get(argv[3]),
			CastAny< Argument5Type >::get(argv[4]),
			CastAny< Argument6Type >::get(argv[5]),
			CastAny< Argument7Type >::get(argv[6])
		);
		return Any();
	}
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type,
	typename Argument6Type,
	typename Argument7Type,
	typename Argument8Type,
	bool Const
>
struct Method_8 : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(Method_8);

	typedef typename MethodSignature< Const, ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type >::method_t method_t;

	method_t m_method;

	Method_8(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss); ss << L","; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss); ss << L","; CastAny< Argument5Type >::typeName(ss); ss << L","; CastAny< Argument6Type >::typeName(ss); ss << L","; CastAny< Argument7Type >::typeName(ss); ss << L","; CastAny< Argument8Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENT_COUNT(8)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)
		T_VERIFY_ARGUMENT_TYPE(1, Argument2Type)
		T_VERIFY_ARGUMENT_TYPE(2, Argument3Type)
		T_VERIFY_ARGUMENT_TYPE(3, Argument4Type)
		T_VERIFY_ARGUMENT_TYPE(4, Argument5Type)
		T_VERIFY_ARGUMENT_TYPE(5, Argument6Type)
		T_VERIFY_ARGUMENT_TYPE(6, Argument7Type)
		T_VERIFY_ARGUMENT_TYPE(7, Argument8Type)

		ReturnType returnValue = (mandatory_non_null_type_cast< ClassType* >(self)->*m_method)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2]),
			CastAny< Argument4Type >::get(argv[3]),
			CastAny< Argument5Type >::get(argv[4]),
			CastAny< Argument6Type >::get(argv[5]),
			CastAny< Argument7Type >::get(argv[6]),
			CastAny< Argument8Type >::get(argv[7])
		);
		return CastAny< ReturnType >::set(returnValue);
	}
};

template <
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type,
	typename Argument6Type,
	typename Argument7Type,
	typename Argument8Type,
	bool Const
>
struct Method_8 < ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type, Const > : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(Method_8);

	typedef typename MethodSignature< Const, ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type >::method_t method_t;

	method_t m_method;

	Method_8(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void,"; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss); ss << L","; CastAny< Argument5Type >::typeName(ss); ss << L","; CastAny< Argument6Type >::typeName(ss); ss << L","; CastAny< Argument7Type >::typeName(ss); ss << L","; CastAny< Argument8Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENT_COUNT(8)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)
		T_VERIFY_ARGUMENT_TYPE(1, Argument2Type)
		T_VERIFY_ARGUMENT_TYPE(2, Argument3Type)
		T_VERIFY_ARGUMENT_TYPE(3, Argument4Type)
		T_VERIFY_ARGUMENT_TYPE(4, Argument5Type)
		T_VERIFY_ARGUMENT_TYPE(5, Argument6Type)
		T_VERIFY_ARGUMENT_TYPE(6, Argument7Type)
		T_VERIFY_ARGUMENT_TYPE(7, Argument8Type)

		(mandatory_non_null_type_cast< ClassType* >(self)->*m_method)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2]),
			CastAny< Argument4Type >::get(argv[3]),
			CastAny< Argument5Type >::get(argv[4]),
			CastAny< Argument6Type >::get(argv[5]),
			CastAny< Argument7Type >::get(argv[6]),
			CastAny< Argument8Type >::get(argv[7])
		);
		return Any();
	}
};

/*! \} */

/*! \} */

}
