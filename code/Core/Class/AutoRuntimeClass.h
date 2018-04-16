/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_AutoScriptClass_H
#define traktor_AutoScriptClass_H

#include "Core/Class/CastAny.h"
#include "Core/Class/IRuntimeClass.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Meta/TypeList.h"

namespace traktor
{

#if !defined(__ANDROID__) && !defined(__IOS__)
#	define T_VERIFY_ARGUMENTS(required) \
		T_FATAL_ASSERT_M (argc >= (required), L"Not enough arguments");
#endif
#if !defined(T_VERIFY_ARGUMENTS)
#	define T_VERIFY_ARGUMENTS(required) 
#endif

/*! \ingroup Core */
/*! \{ */

struct T_NOVTABLE IConstructor
{
	virtual ~IConstructor() {}

	virtual Ref< ITypedObject > construct(uint32_t argc, const Any* argv) const = 0;
};

struct T_NOVTABLE IMethod
{
	virtual ~IMethod() {}

	virtual void signature(OutputStream& ss) const = 0;

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const = 0;
};

struct T_NOVTABLE IStaticMethod
{
	virtual ~IStaticMethod() {}

	virtual void signature(OutputStream& ss) const = 0;

	virtual Any invoke(uint32_t argc, const Any* argv) const = 0;
};

struct T_NOVTABLE IPropertySet
{
	virtual ~IPropertySet() {}

	virtual void invoke(ITypedObject* object, const Any& value) const = 0;
};

struct T_NOVTABLE IPropertyGet
{
	virtual ~IPropertyGet() {}

	virtual Any invoke(ITypedObject* object) const = 0;
};

struct T_NOVTABLE IOperator
{
	virtual ~IOperator() {}

	virtual bool tryPerform(ITypedObject* object, const Any& arg, Any& result) const = 0;
};

/*! \name Constructors */
/*! \{ */

template<
	typename ClassType
>
struct Constructor_0 : public IConstructor
{
	virtual Ref< ITypedObject > construct(uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		return new ClassType();
	}
};

template<
	typename ClassType,
	typename Argument1Type
>
struct Constructor_1 : public IConstructor
{
	virtual Ref< ITypedObject > construct(uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(1)
		return new ClassType(
			CastAny< Argument1Type >::get(argv[0])
		);
	}
};

template<
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type
>
struct Constructor_2 : public IConstructor
{
	virtual Ref< ITypedObject > construct(uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(2)
		return new ClassType(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1])
		);
	}
};

template<
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type
>
struct Constructor_3 : public IConstructor
{
	virtual Ref< ITypedObject > construct(uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(3)
		return new ClassType(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2])
		);
	}
};

template<
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type
>
struct Constructor_4 : public IConstructor
{
	virtual Ref< ITypedObject > construct(uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(4)
		return new ClassType(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2]),
			CastAny< Argument4Type >::get(argv[3])
		);
	}
};

template<
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type
>
struct Constructor_5 : public IConstructor
{
	virtual Ref< ITypedObject > construct(uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(5)
		return new ClassType(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2]),
			CastAny< Argument4Type >::get(argv[3]),
			CastAny< Argument5Type >::get(argv[4])
		);
	}
};

template<
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type,
	typename Argument6Type
>
struct Constructor_6 : public IConstructor
{
	virtual Ref< ITypedObject > construct(uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(6)
		return new ClassType(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2]),
			CastAny< Argument4Type >::get(argv[3]),
			CastAny< Argument5Type >::get(argv[4]),
			CastAny< Argument6Type >::get(argv[5])
		);
	}
};

template<
	typename ClassType,
	typename Argument1Type
>
struct FnConstructor_1 : public IConstructor
{
	T_NO_COPY_CLASS(FnConstructor_1);

	typedef Ref< ClassType > (*fn_t)(Argument1Type);
	fn_t fn;

	FnConstructor_1(fn_t fn_)
	:	fn(fn_)
	{
	}

	virtual Ref< ITypedObject > construct(uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(1)
		return (*fn)(
			CastAny< Argument1Type >::get(argv[0])
		);
	}
};

template<
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type
>
struct FnConstructor_2 : public IConstructor
{
	T_NO_COPY_CLASS(FnConstructor_2);

	typedef Ref< ClassType > (*fn_t)(Argument1Type, Argument2Type);
	fn_t fn;

	FnConstructor_2(fn_t fn_)
	:	fn(fn_)
	{
	}

	virtual Ref< ITypedObject > construct(uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(2)
		return (*fn)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1])
		);
	}
};

template<
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type
>
struct FnConstructor_3 : public IConstructor
{
	T_NO_COPY_CLASS(FnConstructor_3);

	typedef Ref< ClassType > (*fn_t)(Argument1Type, Argument2Type, Argument3Type);
	fn_t fn;

	FnConstructor_3(fn_t fn_)
	:	fn(fn_)
	{
	}

	virtual Ref< ITypedObject > construct(uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(3)
		return (*fn)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2])
		);
	}
};

template<
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type
>
struct FnConstructor_4 : public IConstructor
{
	T_NO_COPY_CLASS(FnConstructor_4);

	typedef Ref< ClassType > (*fn_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type);
	fn_t fn;

	FnConstructor_4(fn_t fn_)
	:	fn(fn_)
	{
	}

	virtual Ref< ITypedObject > construct(uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(4)
		return (*fn)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2]),
			CastAny< Argument4Type >::get(argv[3])
		);
	}
};

/*! \} */

/*! \name Method signatures */
/*! \{ */

template <
	typename ClassType,
	typename ReturnType,
	bool Const
>
struct MethodSignature_0
{
	T_NO_COPY_CLASS(MethodSignature_0);
	typedef ReturnType (ClassType::*method_t)();
	typedef ReturnType (*static_method_t)();
};

template <
	typename ClassType,
	typename ReturnType
>
struct MethodSignature_0 < ClassType, ReturnType, true >
{
	T_NO_COPY_CLASS(MethodSignature_0);
	typedef ReturnType (ClassType::*method_t)() const;
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	bool Const
>
struct MethodSignature_1
{
	T_NO_COPY_CLASS(MethodSignature_1);
	typedef ReturnType (ClassType::*method_t)(Argument1Type);
	typedef ReturnType (*static_method_t)(Argument1Type);
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type
>
struct MethodSignature_1 < ClassType, ReturnType, Argument1Type, true >
{
	T_NO_COPY_CLASS(MethodSignature_1);
	typedef ReturnType (ClassType::*method_t)(Argument1Type) const;
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	bool Const
>
struct MethodSignature_2
{
	T_NO_COPY_CLASS(MethodSignature_2);
	typedef ReturnType (ClassType::*method_t)(Argument1Type, Argument2Type);
	typedef ReturnType (*static_method_t)(Argument1Type, Argument2Type);
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type
>
struct MethodSignature_2 < ClassType, ReturnType, Argument1Type, Argument2Type, true >
{
	T_NO_COPY_CLASS(MethodSignature_2);
	typedef ReturnType (ClassType::*method_t)(Argument1Type, Argument2Type) const;
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	bool Const
>
struct MethodSignature_3
{
	T_NO_COPY_CLASS(MethodSignature_3);
	typedef ReturnType (ClassType::*method_t)(Argument1Type, Argument2Type, Argument3Type);
	typedef ReturnType (*static_method_t)(Argument1Type, Argument2Type, Argument3Type);
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type
>
struct MethodSignature_3 < ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, true >
{
	T_NO_COPY_CLASS(MethodSignature_3);
	typedef ReturnType (ClassType::*method_t)(Argument1Type, Argument2Type, Argument3Type) const;
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
struct MethodSignature_4
{
	T_NO_COPY_CLASS(MethodSignature_4);
	typedef ReturnType (ClassType::*method_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type);
	typedef ReturnType (*static_method_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type);
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type
>
struct MethodSignature_4 < ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, true >
{
	T_NO_COPY_CLASS(MethodSignature_4);
	typedef ReturnType (ClassType::*method_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type) const;
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
struct MethodSignature_5
{
	T_NO_COPY_CLASS(MethodSignature_5);
	typedef ReturnType (ClassType::*method_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type);
	typedef ReturnType (*static_method_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type);
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type
>
struct MethodSignature_5 < ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, true >
{
	T_NO_COPY_CLASS(MethodSignature_5);
	typedef ReturnType (ClassType::*method_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type) const;
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
struct MethodSignature_6
{
	T_NO_COPY_CLASS(MethodSignature_6);
	typedef ReturnType (ClassType::*method_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type);
	typedef ReturnType (*static_method_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type);
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type,
	typename Argument6Type
>
struct MethodSignature_6 < ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, true >
{
	T_NO_COPY_CLASS(MethodSignature_6);
	typedef ReturnType (ClassType::*method_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type) const;
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
struct MethodSignature_7
{
	T_NO_COPY_CLASS(MethodSignature_7);
	typedef ReturnType (ClassType::*method_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type);
	typedef ReturnType (*static_method_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type);
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
	typename Argument7Type
>
struct MethodSignature_7 < ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, true >
{
	T_NO_COPY_CLASS(MethodSignature_7);
	typedef ReturnType (ClassType::*method_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type) const;
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
struct MethodSignature_8
{
	T_NO_COPY_CLASS(MethodSignature_8);
	typedef ReturnType (ClassType::*method_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type);
	typedef ReturnType (*static_method_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type);
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
	typename Argument8Type
>
struct MethodSignature_8 < ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type, true >
{
	T_NO_COPY_CLASS(MethodSignature_8);
	typedef ReturnType (ClassType::*method_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type) const;
};


/*! \} */

/*! \name Method invocations */
/*! \{ */

template <
	typename ClassType,
	typename ReturnType,
	bool Const
>
struct Method_0 : public IMethod
{
	T_NO_COPY_CLASS(Method_0);

	typedef typename MethodSignature_0< ClassType, ReturnType, Const >::method_t method_t;

	method_t m_method;

	Method_0(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		ReturnType returnValue = (mandatory_non_null_type_cast< ClassType* >(object)->*m_method)();
		return CastAny< ReturnType >::set(returnValue);
	}
};

template <
	typename ClassType,
	bool Const
>
struct Method_0 < ClassType, void, Const > : public IMethod
{
	T_NO_COPY_CLASS(Method_0);

	typedef typename MethodSignature_0< ClassType, void, Const >::method_t method_t;

	method_t m_method;

	Method_0(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void";
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		(mandatory_non_null_type_cast< ClassType* >(object)->*m_method)();
		return Any();
	}
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	bool Const
>
struct Method_1 : public IMethod
{
	T_NO_COPY_CLASS(Method_1);

	typedef typename MethodSignature_1< ClassType, ReturnType, Argument1Type, Const >::method_t method_t;

	method_t m_method;

	Method_1(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss); ss << L","; CastAny< Argument1Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(1)
		ReturnType returnValue = (mandatory_non_null_type_cast< ClassType* >(object)->*m_method)(
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
struct Method_1 < ClassType, void, Argument1Type, Const > : public IMethod
{
	T_NO_COPY_CLASS(Method_1);

	typedef typename MethodSignature_1< ClassType, void, Argument1Type, Const >::method_t method_t;

	method_t m_method;

	Method_1(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void,"; CastAny< Argument1Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(1)
		(mandatory_non_null_type_cast< ClassType* >(object)->*m_method)(
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
struct Method_2 : public IMethod
{
	T_NO_COPY_CLASS(Method_2);

	typedef typename MethodSignature_2< ClassType, ReturnType, Argument1Type, Argument2Type, Const >::method_t method_t;

	method_t m_method;

	Method_2(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss); ss << L","; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(2)
		ReturnType returnValue = (mandatory_non_null_type_cast< ClassType* >(object)->*m_method)(
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
struct Method_2 < ClassType, void, Argument1Type, Argument2Type, Const > : public IMethod
{
	T_NO_COPY_CLASS(Method_2);

	typedef typename MethodSignature_2< ClassType, void, Argument1Type, Argument2Type, Const >::method_t method_t;

	method_t m_method;

	Method_2(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void,"; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(2)
		(mandatory_non_null_type_cast< ClassType* >(object)->*m_method)(
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
struct Method_3 : public IMethod
{
	T_NO_COPY_CLASS(Method_3);

	typedef typename MethodSignature_3< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Const >::method_t method_t;

	method_t m_method;

	Method_3(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss); ss << L","; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(3)
		ReturnType returnValue = (mandatory_non_null_type_cast< ClassType* >(object)->*m_method)(
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
struct Method_3 < ClassType, void, Argument1Type, Argument2Type, Argument3Type, Const > : public IMethod
{
	T_NO_COPY_CLASS(Method_3);

	typedef typename MethodSignature_3< ClassType, void, Argument1Type, Argument2Type, Argument3Type, Const >::method_t method_t;

	method_t m_method;

	Method_3(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void,"; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(3)
		(mandatory_non_null_type_cast< ClassType* >(object)->*m_method)(
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
struct Method_4 : public IMethod
{
	T_NO_COPY_CLASS(Method_4);

	typedef typename MethodSignature_4< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Const >::method_t method_t;

	method_t m_method;

	Method_4(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss); ss << L","; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(4)
		ReturnType returnValue = (mandatory_non_null_type_cast< ClassType* >(object)->*m_method)(
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
struct Method_4 < ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Const > : public IMethod
{
	T_NO_COPY_CLASS(Method_4);

	typedef typename MethodSignature_4< ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Const >::method_t method_t;

	method_t m_method;

	Method_4(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void,"; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(4)
		(mandatory_non_null_type_cast< ClassType* >(object)->*m_method)(
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
struct Method_5 : public IMethod
{
	T_NO_COPY_CLASS(Method_5);

	typedef typename MethodSignature_5< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Const >::method_t method_t;

	method_t m_method;

	Method_5(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss); ss << L","; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss); ss << L","; CastAny< Argument5Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(5)
		ReturnType returnValue = (mandatory_non_null_type_cast< ClassType* >(object)->*m_method)(
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
struct Method_5 < ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Const > : public IMethod
{
	T_NO_COPY_CLASS(Method_5);

	typedef typename MethodSignature_5< ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Const >::method_t method_t;

	method_t m_method;

	Method_5(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void,"; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss); ss << L","; CastAny< Argument5Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(5)
		(mandatory_non_null_type_cast< ClassType* >(object)->*m_method)(
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
struct Method_6 : public IMethod
{
	T_NO_COPY_CLASS(Method_6);

	typedef typename MethodSignature_6< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Const >::method_t method_t;

	method_t m_method;

	Method_6(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss); ss << L","; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss); ss << L","; CastAny< Argument5Type >::typeName(ss); ss << L","; CastAny< Argument6Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(6)
		ReturnType returnValue = (mandatory_non_null_type_cast< ClassType* >(object)->*m_method)(
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
struct Method_6 < ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Const > : public IMethod
{
	T_NO_COPY_CLASS(Method_6);

	typedef typename MethodSignature_6< ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Const >::method_t method_t;

	method_t m_method;

	Method_6(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void,"; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss); ss << L","; CastAny< Argument5Type >::typeName(ss); ss << L","; CastAny< Argument6Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(6)
		(mandatory_non_null_type_cast< ClassType* >(object)->*m_method)(
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
struct Method_7 : public IMethod
{
	T_NO_COPY_CLASS(Method_7);

	typedef typename MethodSignature_7< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Const >::method_t method_t;

	method_t m_method;

	Method_7(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss); ss << L","; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss); ss << L","; CastAny< Argument5Type >::typeName(ss); ss << L","; CastAny< Argument6Type >::typeName(ss); ss << L","; CastAny< Argument7Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(7)
		ReturnType returnValue = (mandatory_non_null_type_cast< ClassType* >(object)->*m_method)(
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
struct Method_7 < ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Const > : public IMethod
{
	T_NO_COPY_CLASS(Method_7);

	typedef typename MethodSignature_7< ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Const >::method_t method_t;

	method_t m_method;

	Method_7(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void,"; ss << CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss); ss << L","; CastAny< Argument5Type >::typeName(ss); ss << L","; CastAny< Argument6Type >::typeName(ss); ss << L","; CastAny< Argument7Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(7)
		(mandatory_non_null_type_cast< ClassType* >(object)->*m_method)(
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
struct Method_8 : public IMethod
{
	T_NO_COPY_CLASS(Method_8);

	typedef typename MethodSignature_8< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type, Const >::method_t method_t;

	method_t m_method;

	Method_8(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss); ss << L","; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss); ss << L","; CastAny< Argument5Type >::typeName(ss); ss << L","; CastAny< Argument6Type >::typeName(ss); ss << L","; CastAny< Argument7Type >::typeName(ss); ss << L","; CastAny< Argument8Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(8)
		ReturnType returnValue = (mandatory_non_null_type_cast< ClassType* >(object)->*m_method)(
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
struct Method_8 < ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type, Const > : public IMethod
{
	T_NO_COPY_CLASS(Method_8);

	typedef typename MethodSignature_8< ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type, Const >::method_t method_t;

	method_t m_method;

	Method_8(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void,"; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss); ss << L","; CastAny< Argument5Type >::typeName(ss); ss << L","; CastAny< Argument6Type >::typeName(ss); ss << L","; CastAny< Argument7Type >::typeName(ss); ss << L","; CastAny< Argument8Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(8)
		(mandatory_non_null_type_cast< ClassType* >(object)->*m_method)(
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

/*! \name Method through trunks */
/*! \{ */

template <
	typename ClassType,
	typename ReturnType
>
struct MethodTrunk_0 : public IMethod
{
	T_NO_COPY_CLASS(MethodTrunk_0);

	typedef ReturnType (*method_t)(ClassType*);

	method_t m_method;

	MethodTrunk_0(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		ReturnType returnValue = (*m_method)(mandatory_non_null_type_cast< ClassType* >(object));
		return CastAny< ReturnType >::set(returnValue);
	}
};

template <
	typename ClassType
>
struct MethodTrunk_0< ClassType, void > : public IMethod
{
	T_NO_COPY_CLASS(MethodTrunk_0);

	typedef void (*method_t)(ClassType*);

	method_t m_method;

	MethodTrunk_0(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void";
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		(*m_method)(mandatory_non_null_type_cast< ClassType* >(object));
		return Any();
	}
};


template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type
>
struct MethodTrunk_1 : public IMethod
{
	T_NO_COPY_CLASS(MethodTrunk_1);

	typedef ReturnType (*method_t)(ClassType*, Argument1Type);

	method_t m_method;

	MethodTrunk_1(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss); ss << L","; CastAny< Argument1Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(1)
		ReturnType returnValue = (*m_method)(
			mandatory_non_null_type_cast< ClassType* >(object),
			CastAny< Argument1Type >::get(argv[0])
		);
		return CastAny< ReturnType >::set(returnValue);
	}
};

template <
	typename ClassType,
	typename Argument1Type
>
struct MethodTrunk_1 < ClassType, void, Argument1Type > : public IMethod
{
	T_NO_COPY_CLASS(MethodTrunk_1);

	typedef void (*method_t)(ClassType*, Argument1Type);

	method_t m_method;

	MethodTrunk_1(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void,"; CastAny< Argument1Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(1)
		(*m_method)(
			mandatory_non_null_type_cast< ClassType* >(object),
			CastAny< Argument1Type >::get(argv[0])
		);
		return Any();
	}
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type
>
struct MethodTrunk_2 : public IMethod
{
	T_NO_COPY_CLASS(MethodTrunk_2);

	typedef ReturnType (*method_t)(ClassType*, Argument1Type, Argument2Type);

	method_t m_method;

	MethodTrunk_2(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss); ss << L","; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(2)
		ReturnType returnValue = (*m_method)(
			mandatory_non_null_type_cast< ClassType* >(object),
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1])
		);
		return CastAny< ReturnType >::set(returnValue);
	}
};

template <
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type
>
struct MethodTrunk_2 < ClassType, void, Argument1Type, Argument2Type > : public IMethod
{
	T_NO_COPY_CLASS(MethodTrunk_2);

	typedef void (*method_t)(ClassType*, Argument1Type, Argument2Type);

	method_t m_method;

	MethodTrunk_2(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void,"; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(2)
		(*m_method)(
			mandatory_non_null_type_cast< ClassType* >(object),
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
	typename Argument3Type
>
struct MethodTrunk_3 : public IMethod
{
	T_NO_COPY_CLASS(MethodTrunk_3);

	typedef ReturnType (*method_t)(ClassType*, Argument1Type, Argument2Type, Argument3Type);

	method_t m_method;

	MethodTrunk_3(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss); ss << L","; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(3)
		ReturnType returnValue = (*m_method)(
			mandatory_non_null_type_cast< ClassType* >(object),
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
	typename Argument3Type
>
struct MethodTrunk_3 < ClassType, void, Argument1Type, Argument2Type, Argument3Type > : public IMethod
{
	T_NO_COPY_CLASS(MethodTrunk_3);

	typedef void (*method_t)(ClassType*, Argument1Type, Argument2Type, Argument3Type);

	method_t m_method;

	MethodTrunk_3(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void,"; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(3)
		(*m_method)(
			mandatory_non_null_type_cast< ClassType* >(object),
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
	typename Argument4Type
>
struct MethodTrunk_4 : public IMethod
{
	T_NO_COPY_CLASS(MethodTrunk_4);

	typedef ReturnType (*method_t)(ClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type);

	method_t m_method;

	MethodTrunk_4(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss); ss << L","; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(4)
		ReturnType returnValue = (*m_method)(
			mandatory_non_null_type_cast< ClassType* >(object),
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
	typename Argument4Type
>
struct MethodTrunk_4 < ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type > : public IMethod
{
	T_NO_COPY_CLASS(MethodTrunk_4);

	typedef void (*method_t)(ClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type);

	method_t m_method;

	MethodTrunk_4(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void,"; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(4)
		(*m_method)(
			mandatory_non_null_type_cast< ClassType* >(object),
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
	typename Argument5Type
>
struct MethodTrunk_5 : public IMethod
{
	T_NO_COPY_CLASS(MethodTrunk_5);

	typedef ReturnType (*method_t)(ClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type);

	method_t m_method;

	MethodTrunk_5(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss); ss << L","; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss); ss << L","; CastAny< Argument5Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(5)
		ReturnType returnValue = (*m_method)(
			mandatory_non_null_type_cast< ClassType* >(object),
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
	typename Argument5Type
>
struct MethodTrunk_5 < ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type > : public IMethod
{
	T_NO_COPY_CLASS(MethodTrunk_5);

	typedef void (*method_t)(ClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type);

	method_t m_method;

	MethodTrunk_5(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void,"; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss); ss << L","; CastAny< Argument5Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(5)
		(*m_method)(
			mandatory_non_null_type_cast< ClassType* >(object),
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
	typename Argument6Type
>
struct MethodTrunk_6 : public IMethod
{
	T_NO_COPY_CLASS(MethodTrunk_6);

	typedef ReturnType (*method_t)(ClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type);

	method_t m_method;

	MethodTrunk_6(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss); ss << L","; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss); ss << L","; CastAny< Argument5Type >::typeName(ss); ss << L","; CastAny< Argument6Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(6)
		ReturnType returnValue = (*m_method)(
			mandatory_non_null_type_cast< ClassType* >(object),
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
	typename Argument6Type
>
struct MethodTrunk_6 < ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type > : public IMethod
{
	T_NO_COPY_CLASS(MethodTrunk_6);

	typedef void (*method_t)(ClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type);

	method_t m_method;

	MethodTrunk_6(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void,"; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss); ss << L","; CastAny< Argument5Type >::typeName(ss); ss << L","; CastAny< Argument6Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(6)
		(*m_method)(
			mandatory_non_null_type_cast< ClassType* >(object),
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
	typename Argument7Type
>
struct MethodTrunk_7 : public IMethod
{
	T_NO_COPY_CLASS(MethodTrunk_7);

	typedef ReturnType (*method_t)(ClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type);

	method_t m_method;

	MethodTrunk_7(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss); ss << L","; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss); ss << L","; CastAny< Argument5Type >::typeName(ss); ss << L","; CastAny< Argument6Type >::typeName(ss); ss << L","; CastAny< Argument7Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(7)
		ReturnType returnValue = (*m_method)(
			mandatory_non_null_type_cast< ClassType* >(object),
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
	typename Argument7Type
>
struct MethodTrunk_7 < ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type > : public IMethod
{
	T_NO_COPY_CLASS(MethodTrunk_7);

	typedef void (*method_t)(ClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type);

	method_t m_method;

	MethodTrunk_7(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void,"; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss); ss << L","; CastAny< Argument5Type >::typeName(ss); ss << L","; CastAny< Argument6Type >::typeName(ss); ss << L","; CastAny< Argument7Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(7)
		(*m_method)(
			mandatory_non_null_type_cast< ClassType* >(object),
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
	typename Argument8Type
>
struct MethodTrunk_8 : public IMethod
{
	T_NO_COPY_CLASS(MethodTrunk_8);

	typedef ReturnType (*method_t)(ClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type);

	method_t m_method;

	MethodTrunk_8(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss); ss << L","; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss); ss << L","; CastAny< Argument5Type >::typeName(ss); ss << L","; CastAny< Argument6Type >::typeName(ss); ss << L","; CastAny< Argument7Type >::typeName(ss); ss << L","; CastAny< Argument8Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(8)
		ReturnType returnValue = (*m_method)(
			mandatory_non_null_type_cast< ClassType* >(object),
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
	typename Argument8Type
>
struct MethodTrunk_8 < ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type > : public IMethod
{
	T_NO_COPY_CLASS(MethodTrunk_8);

	typedef void (*method_t)(ClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type);

	method_t m_method;

	MethodTrunk_8(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void,"; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss); ss << L","; CastAny< Argument5Type >::typeName(ss); ss << L","; CastAny< Argument6Type >::typeName(ss); ss << L","; CastAny< Argument7Type >::typeName(ss); ss << L","; CastAny< Argument8Type >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* object, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(8)
		(*m_method)(
			mandatory_non_null_type_cast< ClassType* >(object),
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

/*! \name Static method invocations */
/*! \{ */

template <
	typename ClassType,
	typename ReturnType
>
struct StaticMethod_0 : public IStaticMethod
{
	T_NO_COPY_CLASS(StaticMethod_0);

	typedef typename MethodSignature_0< ClassType, ReturnType, false >::static_method_t static_method_t;

	static_method_t m_method;

	StaticMethod_0(static_method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss);
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		ReturnType returnValue = (*m_method)();
		return CastAny< ReturnType >::set(returnValue);
	}
};

template <
	typename ClassType
>
struct StaticMethod_0 < ClassType, void > : public IStaticMethod
{
	T_NO_COPY_CLASS(StaticMethod_0);

	typedef typename MethodSignature_0< ClassType, void, false >::static_method_t static_method_t;

	static_method_t m_method;

	StaticMethod_0(static_method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void";
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		(*m_method)();
		return Any();
	}
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type
>
struct StaticMethod_1 : public IStaticMethod
{
	T_NO_COPY_CLASS(StaticMethod_1);

	typedef typename MethodSignature_1< ClassType, ReturnType, Argument1Type, false >::static_method_t static_method_t;

	static_method_t m_method;

	StaticMethod_1(static_method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss); ss << L","; CastAny< Argument1Type >::typeName(ss);
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(1)
		ReturnType returnValue = (*m_method)(
			CastAny< Argument1Type >::get(argv[0])
		);
		return CastAny< ReturnType >::set(returnValue);
	}
};

template <
	typename ClassType,
	typename Argument1Type
>
struct StaticMethod_1 < ClassType, void, Argument1Type > : public IStaticMethod
{
	T_NO_COPY_CLASS(StaticMethod_1);

	typedef typename MethodSignature_1< ClassType, void, Argument1Type, false >::static_method_t static_method_t;

	static_method_t m_method;

	StaticMethod_1(static_method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void,"; CastAny< Argument1Type >::typeName(ss);
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(1)
		(*m_method)(
			CastAny< Argument1Type >::get(argv[0])
		);
		return Any();
	}
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type
>
struct StaticMethod_2 : public IStaticMethod
{
	T_NO_COPY_CLASS(StaticMethod_2);

	typedef typename MethodSignature_2< ClassType, ReturnType, Argument1Type, Argument2Type, false >::static_method_t static_method_t;

	static_method_t m_method;

	StaticMethod_2(static_method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss); ss << L","; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss);
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(2)
		ReturnType returnValue = (*m_method)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1])
		);
		return CastAny< ReturnType >::set(returnValue);
	}
};

template <
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type
>
struct StaticMethod_2 < ClassType, void, Argument1Type, Argument2Type > : public IStaticMethod
{
	T_NO_COPY_CLASS(StaticMethod_2);

	typedef typename MethodSignature_2< ClassType, void, Argument1Type, Argument2Type, false >::static_method_t static_method_t;

	static_method_t m_method;

	StaticMethod_2(static_method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void,"; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss);
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(2)
		(*m_method)(
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
	typename Argument3Type
>
struct StaticMethod_3 : public IStaticMethod
{
	T_NO_COPY_CLASS(StaticMethod_3);

	typedef typename MethodSignature_3< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, false >::static_method_t static_method_t;

	static_method_t m_method;

	StaticMethod_3(static_method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss); ss << L","; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss);
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(3)
		ReturnType returnValue = (*m_method)(
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
	typename Argument3Type
>
struct StaticMethod_3 < ClassType, void, Argument1Type, Argument2Type, Argument3Type > : public IStaticMethod
{
	T_NO_COPY_CLASS(StaticMethod_3);

	typedef typename MethodSignature_3< ClassType, void, Argument1Type, Argument2Type, Argument3Type, false >::static_method_t static_method_t;

	static_method_t m_method;

	StaticMethod_3(static_method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void,"; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss);
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(3)
		(*m_method)(
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
	typename Argument4Type
>
struct StaticMethod_4 : public IStaticMethod
{
	T_NO_COPY_CLASS(StaticMethod_4);

	typedef typename MethodSignature_4< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, false >::static_method_t static_method_t;

	static_method_t m_method;

	StaticMethod_4(static_method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss); ss << L","; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss);
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(4)
		ReturnType returnValue = (*m_method)(
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
	typename Argument4Type
>
struct StaticMethod_4 < ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type > : public IStaticMethod
{
	T_NO_COPY_CLASS(StaticMethod_4);

	typedef typename MethodSignature_4< ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, false >::static_method_t static_method_t;

	static_method_t m_method;

	StaticMethod_4(static_method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void,"; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss);
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(4)
		(*m_method)(
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
	typename Argument5Type
>
struct StaticMethod_5 : public IStaticMethod
{
	T_NO_COPY_CLASS(StaticMethod_5);

	typedef typename MethodSignature_5< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, false >::static_method_t static_method_t;

	static_method_t m_method;

	StaticMethod_5(static_method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss); ss << L","; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss); ss << L","; CastAny< Argument5Type >::typeName(ss);
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(5)
		ReturnType returnValue = (*m_method)(
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
	typename Argument5Type
>
struct StaticMethod_5 < ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type > : public IStaticMethod
{
	T_NO_COPY_CLASS(StaticMethod_5);

	typedef typename MethodSignature_5< ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, false >::static_method_t static_method_t;

	static_method_t m_method;

	StaticMethod_5(static_method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void,"; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss); ss << L","; CastAny< Argument5Type >::typeName(ss);
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(5)
		(*m_method)(
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
	typename Argument6Type
>
struct StaticMethod_6 : public IStaticMethod
{
	T_NO_COPY_CLASS(StaticMethod_6);

	typedef typename MethodSignature_6< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, false >::static_method_t static_method_t;

	static_method_t m_method;

	StaticMethod_6(static_method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss); ss << L","; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss); ss << L","; CastAny< Argument5Type >::typeName(ss); ss << L","; CastAny< Argument6Type >::typeName(ss);
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(6)
		ReturnType returnValue = (*m_method)(
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
	typename Argument6Type
>
struct StaticMethod_6 < ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type > : public IStaticMethod
{
	T_NO_COPY_CLASS(StaticMethod_6);

	typedef typename MethodSignature_6< ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, false >::static_method_t static_method_t;

	static_method_t m_method;

	StaticMethod_6(static_method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void,"; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss); ss << L","; CastAny< Argument5Type >::typeName(ss); ss << L","; CastAny< Argument6Type >::typeName(ss);
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(6)
		(*m_method)(
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
	typename Argument7Type
>
struct StaticMethod_7 : public IStaticMethod
{
	T_NO_COPY_CLASS(StaticMethod_7);

	typedef typename MethodSignature_7< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, false >::static_method_t static_method_t;

	static_method_t m_method;

	StaticMethod_7(static_method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		CastAny< ReturnType >::typeName(ss); ss << L","; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss); ss << L","; CastAny< Argument5Type >::typeName(ss); ss << L","; CastAny< Argument6Type >::typeName(ss); ss << L","; CastAny< Argument7Type >::typeName(ss);
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(7)
		ReturnType returnValue = (*m_method)(
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
	typename Argument7Type
>
struct StaticMethod_7 < ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type > : public IStaticMethod
{
	T_NO_COPY_CLASS(StaticMethod_7);

	typedef typename MethodSignature_7< ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, false >::static_method_t static_method_t;

	static_method_t m_method;

	StaticMethod_7(static_method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
		ss << L"void,"; CastAny< Argument1Type >::typeName(ss); ss << L","; CastAny< Argument2Type >::typeName(ss); ss << L","; CastAny< Argument3Type >::typeName(ss); ss << L","; CastAny< Argument4Type >::typeName(ss); ss << L","; CastAny< Argument5Type >::typeName(ss); ss << L","; CastAny< Argument6Type >::typeName(ss); ss << L","; CastAny< Argument7Type >::typeName(ss);
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENTS(7)
		(*m_method)(
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

/*! \} */

/*! \name Properties */
/*! \{ */

template <
	typename ClassType,
	typename ValueType
>
struct PropertySet : public IPropertySet
{
	T_NO_COPY_CLASS(PropertySet);

	typedef typename MethodSignature_1< ClassType, void, ValueType, false >::method_t method_t;

	method_t m_method;

	PropertySet(method_t method)
	:	m_method(method)
	{
	}

	virtual void invoke(ITypedObject* object, const Any& value) const T_OVERRIDE T_FINAL
	{
		(mandatory_non_null_type_cast< ClassType* >(object)->*m_method)(
			CastAny< ValueType >::get(value)
		);
	}
};

template <
	typename ClassType,
	typename ValueType,
	bool Const
>
struct PropertyGet : public IPropertyGet
{
	T_NO_COPY_CLASS(PropertyGet);

	typedef typename MethodSignature_0< ClassType, ValueType, Const >::method_t method_t;

	method_t m_method;

	PropertyGet(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(ITypedObject* object) const T_OVERRIDE T_FINAL
	{
		ValueType value = (mandatory_non_null_type_cast< ClassType* >(object)->*m_method)();
		return CastAny< ValueType >::set(value);
	}
};

template <
	typename ClassType,
	typename ValueType
>
struct FnPropertySet : public IPropertySet
{
	T_NO_COPY_CLASS(FnPropertySet);

	typedef void (*method_t)(ClassType*, ValueType);

	method_t m_method;

	FnPropertySet(method_t method)
	:	m_method(method)
	{
	}

	virtual void invoke(ITypedObject* object, const Any& value) const T_OVERRIDE T_FINAL
	{
		(*m_method)(
			mandatory_non_null_type_cast< ClassType* >(object),
			CastAny< ValueType >::get(value)
		);
	}
};

template <
	typename ClassType,
	typename ValueType
>
struct FnPropertyGet : public IPropertyGet
{
	T_NO_COPY_CLASS(FnPropertyGet);

	typedef ValueType (*method_t)(ClassType*);

	method_t m_method;

	FnPropertyGet(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(ITypedObject* object) const T_OVERRIDE T_FINAL
	{
		ValueType value = (*m_method)(mandatory_non_null_type_cast< ClassType* >(object));
		return CastAny< ValueType >::set(value);
	}
};

/*! \} */

/*! \name Operator */
/*! \{ */

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type
>
struct Operator : public IOperator
{
	T_NO_COPY_CLASS(Operator);

	typedef typename MethodSignature_1< ClassType, ReturnType, Argument1Type, true >::method_t method_t;

	method_t m_method;

	Operator(method_t method)
	:	m_method(method)
	{
	}

	virtual bool tryPerform(ITypedObject* object, const Any& arg, Any& result) const T_OVERRIDE T_FINAL
	{
		if (CastAny< Argument1Type >::accept(arg))
		{
			ClassType* target = mandatory_non_null_type_cast< ClassType* >(object);
			result = CastAny< ReturnType >::set((target->*m_method)(
				CastAny< Argument1Type >::get(arg)
			));
			return true;
		}
		else
			return false;
	}
};

/*! \} */

/*! \brief Automatic generation of script class definition.
 * \ingroup Core
 *
 * This class simplifies code necessary to map native classes
 * into script classes.
 * Just call addMethod with a pointer to your method and
 * it will automatically generate a "invoke" stub in compile
 * time.
 */
template < typename ClassType >
class AutoRuntimeClass : public IRuntimeClass
{
public:
	T_NO_COPY_CLASS(AutoRuntimeClass);

	typedef Any (*unknown_fn_t)(ClassType* self, const std::string& methodName, uint32_t argc, const Any* argv);

	AutoRuntimeClass()
	:	m_unknown(0)
	{
	}

	virtual ~AutoRuntimeClass()
	{
		T_EXCEPTION_GUARD_BEGIN
		
		for (std::vector< IConstructor* >::iterator i = m_constructors.begin(); i != m_constructors.end(); ++i)
			delete *i;
		
		for (typename std::vector< MethodInfo >::iterator i = m_methods.begin(); i != m_methods.end(); ++i)
		{
			for (std::vector< IMethod* >::iterator j = i->methods.begin(); j != i->methods.end(); ++j)
				delete *j;
		}

		for (typename std::vector< StaticMethodInfo >::iterator i = m_staticMethods.begin(); i != m_staticMethods.end(); ++i)
		{
			for (std::vector< IStaticMethod* >::iterator j = i->methods.begin(); j != i->methods.end(); ++j)
				delete *j;
		}

		for (int i = 0; i < sizeof_array(m_operators); ++i)
		{
			for (std::vector< IOperator* >::iterator j = m_operators[i].begin(); j != m_operators[i].end(); ++j)
				delete *j;
		}

		T_EXCEPTION_GUARD_END
	}

	/*! \name Constructors */
	/*! \{ */

	void addConstructor()
	{
		addConstructor(0, new Constructor_0< ClassType >());
	}

	template <
		typename Argument1Type
	>
	void addConstructor()
	{
		addConstructor(1, new Constructor_1< ClassType, Argument1Type >());
	}

	template <
		typename Argument1Type,
		typename Argument2Type
	>
	void addConstructor()
	{
		addConstructor(2, new Constructor_2< ClassType, Argument1Type, Argument2Type >());
	}

	template <
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type
	>
	void addConstructor()
	{
		addConstructor(3, new Constructor_3< ClassType, Argument1Type, Argument2Type, Argument3Type >());
	}

	template <
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type
	>
	void addConstructor()
	{
		addConstructor(4, new Constructor_4< ClassType, Argument1Type, Argument2Type, Argument3Type, Argument4Type >());
	}

	template <
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type,
		typename Argument5Type
	>
	void addConstructor()
	{
		addConstructor(5, new Constructor_5< ClassType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type >());
	}

	template <
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type,
		typename Argument5Type,
		typename Argument6Type
	>
	void addConstructor()
	{
		addConstructor(6, new Constructor_6< ClassType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type >());
	}

	template <
		typename Argument1Type
	>
	void addConstructor(typename FnConstructor_1< ClassType, Argument1Type >::fn_t fn)
	{
		addConstructor(1, new FnConstructor_1< ClassType, Argument1Type >(fn));
	}

	template <
		typename Argument1Type,
		typename Argument2Type
	>
	void addConstructor(typename FnConstructor_2< ClassType, Argument1Type, Argument2Type >::fn_t fn)
	{
		addConstructor(2, new FnConstructor_2< ClassType, Argument1Type, Argument2Type >(fn));
	}

	template <
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type
	>
	void addConstructor(typename FnConstructor_3< ClassType, Argument1Type, Argument2Type, Argument3Type >::fn_t fn)
	{
		addConstructor(3, new FnConstructor_3< ClassType, Argument1Type, Argument2Type, Argument3Type >(fn));
	}

	template <
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type
	>
	void addConstructor(typename FnConstructor_4< ClassType, Argument1Type, Argument2Type, Argument3Type, Argument4Type >::fn_t fn)
	{
		addConstructor(4, new FnConstructor_4< ClassType, Argument1Type, Argument2Type, Argument3Type, Argument4Type >(fn));
	}

	/*! \} */

	/*! \name Constants */
	/*! \{ */

	void addConstant(const std::string& name, const Any& value)
	{
		ConstInfo ci;
		ci.name = name;
		ci.value = value;
		m_consts.push_back(ci);
	}

	/*! \} */

	/*! \name Methods */
	/*! \{ */

	template <
		typename ReturnType
	>
	void addMethod(const std::string& methodName, ReturnType (ClassType::*method)())
	{
		addMethod(methodName, 0, new Method_0< ClassType, ReturnType, false >(method));
	}

	template <
		typename ReturnType
	>
	void addMethod(const std::string& methodName, ReturnType (ClassType::*method)() const)
	{
		addMethod(methodName, 0, new Method_0< ClassType, ReturnType, true >(method));
	}

	template <
		typename ReturnType
	>
	void addMethod(const std::string& methodName, ReturnType (*method)(ClassType*))
	{
		addMethod(methodName, 0, new MethodTrunk_0< ClassType, ReturnType >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type
	>
	void addMethod(const std::string& methodName, ReturnType (ClassType::*method)(Argument1Type))
	{
		addMethod(methodName, 1, new Method_1< ClassType, ReturnType, Argument1Type, false >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type
	>
	void addMethod(const std::string& methodName, ReturnType (ClassType::*method)(Argument1Type) const)
	{
		addMethod(methodName, 1, new Method_1< ClassType, ReturnType, Argument1Type, true >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type
	>
	void addMethod(const std::string& methodName, ReturnType (*method)(ClassType*, Argument1Type))
	{
		addMethod(methodName, 1, new MethodTrunk_1< ClassType, ReturnType, Argument1Type >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type
	>
	void addMethod(const std::string& methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type))
	{
		addMethod(methodName, 2, new Method_2< ClassType, ReturnType, Argument1Type, Argument2Type, false >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type
	>
	void addMethod(const std::string& methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type) const)
	{
		addMethod(methodName, 2, new Method_2< ClassType, ReturnType, Argument1Type, Argument2Type, true >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type
	>
	void addMethod(const std::string& methodName, ReturnType (*method)(ClassType*, Argument1Type, Argument2Type))
	{
		addMethod(methodName, 2, new MethodTrunk_2< ClassType, ReturnType, Argument1Type, Argument2Type >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type
	>
	void addMethod(const std::string& methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type))
	{
		addMethod(methodName, 3, new Method_3< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, false >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type
	>
	void addMethod(const std::string& methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type) const)
	{
		addMethod(methodName, 3, new Method_3< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, true >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type
	>
	void addMethod(const std::string& methodName, ReturnType (*method)(ClassType*, Argument1Type, Argument2Type, Argument3Type))
	{
		addMethod(methodName, 3, new MethodTrunk_3< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type
	>
	void addMethod(const std::string& methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type))
	{
		addMethod(methodName, 4, new Method_4< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, false >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type
	>
	void addMethod(const std::string& methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type) const)
	{
		addMethod(methodName, 4, new Method_4< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, true >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type
	>
	void addMethod(const std::string& methodName, ReturnType (*method)(ClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type))
	{
		addMethod(methodName, 4, new MethodTrunk_4< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type,
		typename Argument5Type
	>
	void addMethod(const std::string& methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type))
	{
		addMethod(methodName, 5, new Method_5< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, false >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type,
		typename Argument5Type
	>
	void addMethod(const std::string& methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type) const)
	{
		addMethod(methodName, 5, new Method_5< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, true >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type,
		typename Argument5Type
	>
	void addMethod(const std::string& methodName, ReturnType (*method)(ClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type))
	{
		addMethod(methodName, 5, new MethodTrunk_5< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type,
		typename Argument5Type,
		typename Argument6Type
	>
	void addMethod(const std::string& methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type))
	{
		addMethod(methodName, 6, new Method_6< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, false >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type,
		typename Argument5Type,
		typename Argument6Type
	>
	void addMethod(const std::string& methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type) const)
	{
		addMethod(methodName, 6, new Method_6< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, true >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type,
		typename Argument5Type,
		typename Argument6Type
	>
	void addMethod(const std::string& methodName, ReturnType (*method)(ClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type))
	{
		addMethod(methodName, 6, new MethodTrunk_6< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type,
		typename Argument5Type,
		typename Argument6Type,
		typename Argument7Type
	>
	void addMethod(const std::string& methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type))
	{
		addMethod(methodName, 7, new Method_7< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, false >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type,
		typename Argument5Type,
		typename Argument6Type,
		typename Argument7Type
	>
	void addMethod(const std::string& methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type) const)
	{
		addMethod(methodName, 7, new Method_7< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, true >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type,
		typename Argument5Type,
		typename Argument6Type,
		typename Argument7Type
	>
	void addMethod(const std::string& methodName, ReturnType (*method)(ClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type))
	{
		addMethod(methodName, 7, new MethodTrunk_7< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type,
		typename Argument5Type,
		typename Argument6Type,
		typename Argument7Type,
		typename Argument8Type
	>
	void addMethod(const std::string& methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type))
	{
		addMethod(methodName, 8, new Method_8< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type, false >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type,
		typename Argument5Type,
		typename Argument6Type,
		typename Argument7Type,
		typename Argument8Type
	>
	void addMethod(const std::string& methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type) const)
	{
		addMethod(methodName, 8, new Method_8< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type, true >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type,
		typename Argument5Type,
		typename Argument6Type,
		typename Argument7Type,
		typename Argument8Type
	>
	void addMethod(const std::string& methodName, ReturnType (*method)(ClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type))
	{
		addMethod(methodName, 8, new MethodTrunk_8< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type >(method));
	}

	void setUnknownHandler(unknown_fn_t unknown)
	{
		m_unknown = unknown;
	}

	/*! \} */

	/*! \name Static methods */
	/*! \{ */

	template <
		typename ReturnType
	>
	void addStaticMethod(const std::string& methodName, ReturnType (*method)())
	{
		addStaticMethod(methodName, 0, new StaticMethod_0< ClassType, ReturnType >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type
	>
	void addStaticMethod(const std::string& methodName, ReturnType (*method)(Argument1Type))
	{
		addStaticMethod(methodName, 1, new StaticMethod_1< ClassType, ReturnType, Argument1Type >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type
	>
	void addStaticMethod(const std::string& methodName, ReturnType (*method)(Argument1Type, Argument2Type))
	{
		addStaticMethod(methodName, 2, new StaticMethod_2< ClassType, ReturnType, Argument1Type, Argument2Type >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type
	>
	void addStaticMethod(const std::string& methodName, ReturnType (*method)(Argument1Type, Argument2Type, Argument3Type))
	{
		addStaticMethod(methodName, 3, new StaticMethod_3< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type
	>
	void addStaticMethod(const std::string& methodName, ReturnType (*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type))
	{
		addStaticMethod(methodName, 4, new StaticMethod_4< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type,
		typename Argument5Type
	>
	void addStaticMethod(const std::string& methodName, ReturnType (*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type))
	{
		addStaticMethod(methodName, 5, new StaticMethod_5< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type,
		typename Argument5Type,
		typename Argument6Type
	>
	void addStaticMethod(const std::string& methodName, ReturnType (*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type))
	{
		addStaticMethod(methodName, 6, new StaticMethod_6< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type,
		typename Argument5Type,
		typename Argument6Type,
		typename Argument7Type
	>
	void addStaticMethod(const std::string& methodName, ReturnType (*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type))
	{
		addStaticMethod(methodName, 7, new StaticMethod_7< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type >(method));
	}

	/*! \} */

	template < typename ValueType >
	void addProperty(const std::string& propertyName, ValueType (ClassType::*getter)() const)
	{
		StringOutputStream ss;
		CastAny< ValueType >::typeName(ss);

		addProperty(
			propertyName,
			ss.str(),
			0,
			getter != 0 ? new PropertyGet< ClassType, ValueType, true >(getter) : 0
		);
	}

	template < typename SetterValueType, typename GetterValueType >
	void addProperty(const std::string& propertyName, void (ClassType::*setter)(SetterValueType value), GetterValueType (ClassType::*getter)() const)
	{
		StringOutputStream ss;
		CastAny< GetterValueType >::typeName(ss);

		addProperty(
			propertyName,
			ss.str(),
			setter != 0 ? new PropertySet< ClassType, SetterValueType >(setter) : 0,
			getter != 0 ? new PropertyGet< ClassType, GetterValueType, true >(getter) : 0
		);
	}

	template < typename ValueType >
	void addProperty(const std::string& propertyName, ValueType (ClassType::*getter)())
	{
		StringOutputStream ss;
		CastAny< ValueType >::typeName(ss);

		addProperty(
			propertyName,
			ss.str(),
			0,
			getter != 0 ? new PropertyGet< ClassType, ValueType, false >(getter) : 0
		);
	}

	template < typename SetterValueType, typename GetterValueType >
	void addProperty(const std::string& propertyName, void (ClassType::*setter)(SetterValueType value), GetterValueType (ClassType::*getter)())
	{
		StringOutputStream ss;
		CastAny< GetterValueType >::typeName(ss);

		addProperty(
			propertyName,
			ss.str(),
			setter != 0 ? new PropertySet< ClassType, SetterValueType >(setter) : 0,
			getter != 0 ? new PropertyGet< ClassType, GetterValueType, false >(getter) : 0
		);
	}

	template < typename ValueType >
	void addProperty(const std::string& propertyName, ValueType (*getter)(ClassType* self))
	{
		StringOutputStream ss;
		CastAny< ValueType >::typeName(ss);

		addProperty(
			propertyName,
			ss.str(),
			0,
			getter != 0 ? new FnPropertyGet< ClassType, ValueType >(getter) : 0
		);
	}

	template < typename SetterValueType, typename GetterValueType >
	void addProperty(const std::string& propertyName, void (*setter)(ClassType* self, SetterValueType value), GetterValueType (*getter)(ClassType* self))
	{
		StringOutputStream ss;
		CastAny< GetterValueType >::typeName(ss);

		addProperty(
			propertyName,
			ss.str(),
			setter != 0 ? new FnPropertySet< ClassType, SetterValueType >(setter) : 0,
			getter != 0 ? new FnPropertyGet< ClassType, GetterValueType >(getter) : 0
		);
	}

	template <
		typename ReturnType,
		typename Argument1Type
	>
	void addOperator(char operation, ReturnType (ClassType::*method)(Argument1Type) const)
	{
		IOperator* handler = new Operator< ClassType, ReturnType, Argument1Type >(method);
		switch (operation)
		{
		case '+':
			m_operators[0].push_back(handler);
			break;
		case '-':
			m_operators[1].push_back(handler);
			break;
		case '*':
			m_operators[2].push_back(handler);
			break;
		case '/':
			m_operators[3].push_back(handler);
			break;
		default:
			T_FATAL_ERROR;
			break;
		}
	}

	virtual const TypeInfo& getExportType() const T_OVERRIDE T_FINAL
	{
		return type_of< ClassType >();
	}

	virtual bool haveConstructor() const T_OVERRIDE T_FINAL
	{
		return !m_constructors.empty();
	}

	virtual bool haveUnknown() const T_OVERRIDE T_FINAL
	{
		return m_unknown != 0;
	}

	virtual Ref< ITypedObject > construct(ITypedObject* self, uint32_t argc, const Any* argv, const prototype_t& proto) const T_OVERRIDE T_FINAL
	{
		if (argc < m_constructors.size() && m_constructors[argc] != 0)
			return m_constructors[argc]->construct(argc, argv);
		else
			return 0;
	}

	virtual uint32_t getConstantCount() const T_OVERRIDE T_FINAL
	{
		return uint32_t(m_consts.size());
	}

	virtual std::string getConstantName(uint32_t constId) const T_OVERRIDE T_FINAL
	{
		return m_consts[constId].name;
	}

	virtual Any getConstantValue(uint32_t constId) const T_OVERRIDE T_FINAL
	{
		return m_consts[constId].value;
	}

	virtual uint32_t getMethodCount() const T_OVERRIDE T_FINAL
	{
		return uint32_t(m_methods.size());
	}

	virtual std::string getMethodName(uint32_t methodId) const T_OVERRIDE T_FINAL
	{
		return m_methods[methodId].name;
	}

	virtual std::wstring getMethodSignature(uint32_t methodId) const T_OVERRIDE T_FINAL
	{
		const MethodInfo& info = m_methods[methodId];
		for (std::vector< IMethod* >::const_iterator i = info.methods.begin(); i != info.methods.end(); ++i)
		{
			if (*i)
			{
				StringOutputStream ss;
				(*i)->signature(ss);
				return ss.str();
			}
		}
		return L"";
	}

	virtual Any invoke(ITypedObject* object, uint32_t methodId, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		const MethodInfo& info = m_methods[methodId];
		const std::vector< IMethod* >& methods = info.methods;
		if (argc < methods.size() && methods[argc] != 0)
			return methods[argc]->invoke(object, argc, argv);
		else
		{
			StringOutputStream ss;
			ss << L"Incorrect number of arguments when calling method \"" << mbstows(info.name) << L"\"";
			T_FATAL_ASSERT_M(false, ss.str().c_str());
			return Any();
		}
	}

	virtual uint32_t getStaticMethodCount() const T_OVERRIDE T_FINAL
	{
		return uint32_t(m_staticMethods.size());
	}

	virtual std::string getStaticMethodName(uint32_t methodId) const T_OVERRIDE T_FINAL
	{
		return m_staticMethods[methodId].name;
	}

	virtual std::wstring getStaticMethodSignature(uint32_t methodId) const T_OVERRIDE T_FINAL
	{
		const StaticMethodInfo& info = m_staticMethods[methodId];
		for (std::vector< IStaticMethod* >::const_iterator i = info.methods.begin(); i != info.methods.end(); ++i)
		{
			if (*i)
			{
				StringOutputStream ss;
				(*i)->signature(ss);
				return ss.str();
			}
		}
		return L"";
	}

	virtual Any invokeStatic(uint32_t methodId, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		const StaticMethodInfo& info = m_staticMethods[methodId];
		const std::vector< IStaticMethod* >& methods = info.methods;
		if (argc < methods.size() && methods[argc] != 0)
			return methods[argc]->invoke(argc, argv);
		else
		{
			StringOutputStream ss;
			ss << L"Incorrect number of arguments when calling static method \"" << mbstows(info.name) << L"\"";
			T_FATAL_ASSERT_M(false, ss.str().c_str());
			return Any();
		}
	}

	virtual uint32_t getPropertiesCount() const T_OVERRIDE T_FINAL
	{
		return uint32_t(m_properties.size());
	}

	virtual std::string getPropertyName(uint32_t propertyId) const T_OVERRIDE T_FINAL
	{
		return m_properties[propertyId].name;
	}

	virtual std::wstring getPropertySignature(uint32_t propertyId) const T_OVERRIDE T_FINAL
	{
		return m_properties[propertyId].signature;
	}
	
	virtual Any invokePropertyGet(ITypedObject* self, uint32_t propertyId) const T_OVERRIDE T_FINAL
	{
		if (m_properties[propertyId].getter)
			return m_properties[propertyId].getter->invoke(self);
		else
			return Any();
	}

	virtual void invokePropertySet(ITypedObject* self, uint32_t propertyId, const Any& value) const T_OVERRIDE T_FINAL
	{
		if (m_properties[propertyId].setter)
			m_properties[propertyId].setter->invoke(self, value);
	}

	virtual Any invokeUnknown(ITypedObject* object, const std::string& methodName, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		if (m_unknown)
			return (*m_unknown)(mandatory_non_null_type_cast< ClassType* >(object), methodName, argc, argv);
		else
		{
			StringOutputStream ss;
			ss << L"No such method \"" << mbstows(methodName) << L"\"";
			T_FATAL_ASSERT_M(false, ss.str().c_str());
			return Any();
		}
	}

	virtual Any invokeOperator(ITypedObject* object, uint8_t operation, const Any& arg) const T_OVERRIDE T_FINAL
	{
		Any result;

		// Try evaluate operation through handlers; first handler which return true
		// indicating successful evaluation aborts loop.
		const std::vector< IOperator* >& handlers = m_operators[operation];
		for (std::vector< IOperator* >::const_iterator i = handlers.begin(); i != handlers.end(); ++i)
		{
			if ((*i)->tryPerform(object, arg, result))
				break;
		}

		return result;
	}

private:
	struct ConstInfo
	{
		std::string name;
		Any value;
	};

	struct MethodInfo
	{
		std::string name;
		std::vector< IMethod* > methods;
	};

	struct StaticMethodInfo
	{
		std::string name;
		std::vector< IStaticMethod* > methods;
	};

	struct PropertyInfo
	{
		std::string name;
		std::wstring signature;
		IPropertySet* setter;
		IPropertyGet* getter;
	};

	std::vector< IConstructor* > m_constructors;
	std::vector< ConstInfo > m_consts;
	std::vector< MethodInfo > m_methods;
	std::vector< StaticMethodInfo > m_staticMethods;
	std::vector< PropertyInfo > m_properties;
	std::vector< IOperator* > m_operators[4];
	unknown_fn_t m_unknown;

	void addConstructor(size_t argc, IConstructor* constructor)
	{
		if (m_constructors.size() < argc + 1)
			m_constructors.resize(argc + 1, 0);
		m_constructors[argc] = constructor;
	}

	void addMethod(const std::string& methodName, size_t argc, IMethod* method)
	{
		for (typename std::vector< MethodInfo >::iterator i = m_methods.begin(); i != m_methods.end(); ++i)
		{
			if (i->name == methodName)
			{
				std::vector< IMethod* >& methods = i->methods;
				if (methods.size() < argc + 1)
					methods.resize(argc + 1, 0);
				methods[argc] = method;
				return;
			}
		}

		MethodInfo m;
		m.name = methodName;
		m.methods.resize(argc + 1, 0);
		m.methods[argc] = method;
		m_methods.push_back(m);
	}

	void addStaticMethod(const std::string& methodName, size_t argc, IStaticMethod* method)
	{
		for (typename std::vector< StaticMethodInfo >::iterator i = m_staticMethods.begin(); i != m_staticMethods.end(); ++i)
		{
			if (i->name == methodName)
			{
				std::vector< IStaticMethod* >& methods = i->methods;
				if (methods.size() < argc + 1)
					methods.resize(argc + 1, 0);
				methods[argc] = method;
				return;
			}
		}

		StaticMethodInfo m;
		m.name = methodName;
		m.methods.resize(argc + 1, 0);
		m.methods[argc] = method;
		m_staticMethods.push_back(m);
	}

	void addProperty(const std::string& propertyName, const std::wstring& signature, IPropertySet* setter, IPropertyGet* getter)
	{
		for (typename std::vector< PropertyInfo >::iterator i = m_properties.begin(); i != m_properties.end(); ++i)
		{
			if (i->name == propertyName)
			{
				i->setter = setter;
				i->getter = getter;
				return;
			}
		}

		PropertyInfo p;
		p.name = propertyName;
		p.signature = signature;
		p.setter = setter;
		p.getter = getter;
		m_properties.push_back(p);
	}
};

//@}

}

#endif	// traktor_AutoScriptClass_H
