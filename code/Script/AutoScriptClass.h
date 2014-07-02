#ifndef traktor_script_AutoScriptClass_H
#define traktor_script_AutoScriptClass_H

#include "Core/Meta/TypeList.h"
#include "Script/CastAny.h"
#include "Script/IScriptClass.h"

namespace traktor
{
	namespace script
	{

/*! \ingroup Script */
/*! \{ */

struct T_NOVTABLE IConstructor
{
	virtual ~IConstructor() {}

	virtual Ref< Object > construct(uint32_t argc, const Any* argv) const = 0;
};

struct T_NOVTABLE IMethod
{
	virtual ~IMethod() {}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const = 0;
};

struct T_NOVTABLE IStaticMethod
{
	virtual ~IStaticMethod() {}

	virtual Any invoke(uint32_t argc, const Any* argv) const = 0;
};

struct T_NOVTABLE IOperator
{
	virtual ~IOperator() {}

	virtual bool tryPerform(Object* object, const Any& arg, Any& result) const = 0;
};

/*! \name Constructors */
/*! \{ */

template<
	typename ClassType
>
struct Constructor_0 : public IConstructor
{
	virtual Ref< Object > construct(uint32_t argc, const Any* argv) const
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
	virtual Ref< Object > construct(uint32_t argc, const Any* argv) const
	{
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
	virtual Ref< Object > construct(uint32_t argc, const Any* argv) const
	{
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
	virtual Ref< Object > construct(uint32_t argc, const Any* argv) const
	{
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
	virtual Ref< Object > construct(uint32_t argc, const Any* argv) const
	{
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
	virtual Ref< Object > construct(uint32_t argc, const Any* argv) const
	{
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
	virtual Ref< Object > construct(uint32_t argc, const Any* argv) const
	{
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
	typedef Ref< ClassType > (*fn_t)(Argument1Type a1);
	fn_t fn;

	FnConstructor_1(fn_t fn_)
	:	fn(fn_)
	{
	}

	virtual Ref< Object > construct(uint32_t argc, const Any* argv) const
	{
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
	typedef Ref< ClassType > (*fn_t)(Argument1Type a1, Argument2Type a2);
	fn_t fn;

	FnConstructor_2(fn_t fn_)
	:	fn(fn_)
	{
	}

	virtual Ref< Object > construct(uint32_t argc, const Any* argv) const
	{
		return (*fn)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1])
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
	typedef ReturnType (ClassType::*method_t)();
	typedef ReturnType (*static_method_t)();
};

template <
	typename ClassType,
	typename ReturnType
>
struct MethodSignature_0 < ClassType, ReturnType, true >
{
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
	typedef ReturnType (ClassType::*method_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type) const;
};

template <
	typename ClassType,
	typename ReturnType,
	bool Const
>
struct MethodSignature_Variadic
{
	typedef ReturnType (ClassType::*method_t)(uint32_t argc, const Any* argv);
	typedef ReturnType (*static_method_t)(uint32_t argc, const Any* argv);
};

template <
	typename ClassType,
	typename ReturnType
>
struct MethodSignature_Variadic < ClassType, ReturnType, true >
{
	typedef ReturnType (ClassType::*method_t)(uint32_t argc, const Any* argv) const;
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
	typedef typename MethodSignature_0< ClassType, ReturnType, Const >::method_t method_t;

	method_t m_method;

	Method_0(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		ReturnType returnValue = (checked_type_cast< ClassType*, false >(object)->*m_method)();
		return CastAny< ReturnType >::set(returnValue);
	}
};

template <
	typename ClassType,
	bool Const
>
struct Method_0 < ClassType, void, Const > : public IMethod
{
	typedef typename MethodSignature_0< ClassType, void, Const >::method_t method_t;

	method_t m_method;

	Method_0(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		(checked_type_cast< ClassType*, false >(object)->*m_method)();
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
	typedef typename MethodSignature_1< ClassType, ReturnType, Argument1Type, Const >::method_t method_t;

	method_t m_method;

	Method_1(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		ReturnType returnValue = (checked_type_cast< ClassType*, false >(object)->*m_method)(
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
	typedef typename MethodSignature_1< ClassType, void, Argument1Type, Const >::method_t method_t;

	method_t m_method;

	Method_1(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		(checked_type_cast< ClassType*, false >(object)->*m_method)(
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
	typedef typename MethodSignature_2< ClassType, ReturnType, Argument1Type, Argument2Type, Const >::method_t method_t;

	method_t m_method;

	Method_2(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		ReturnType returnValue = (checked_type_cast< ClassType*, false >(object)->*m_method)(
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
	typedef typename MethodSignature_2< ClassType, void, Argument1Type, Argument2Type, Const >::method_t method_t;

	method_t m_method;

	Method_2(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		(checked_type_cast< ClassType*, false >(object)->*m_method)(
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
	typedef typename MethodSignature_3< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Const >::method_t method_t;

	method_t m_method;

	Method_3(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		ReturnType returnValue = (checked_type_cast< ClassType*, false >(object)->*m_method)(
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
	typedef typename MethodSignature_3< ClassType, void, Argument1Type, Argument2Type, Argument3Type, Const >::method_t method_t;

	method_t m_method;

	Method_3(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		(checked_type_cast< ClassType*, false >(object)->*m_method)(
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
	typedef typename MethodSignature_4< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Const >::method_t method_t;

	method_t m_method;

	Method_4(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		ReturnType returnValue = (checked_type_cast< ClassType*, false >(object)->*m_method)(
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
	typedef typename MethodSignature_4< ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Const >::method_t method_t;

	method_t m_method;

	Method_4(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		(checked_type_cast< ClassType*, false >(object)->*m_method)(
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
	typedef typename MethodSignature_5< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Const >::method_t method_t;

	method_t m_method;

	Method_5(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		ReturnType returnValue = (checked_type_cast< ClassType*, false >(object)->*m_method)(
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
	typedef typename MethodSignature_5< ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Const >::method_t method_t;

	method_t m_method;

	Method_5(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		(checked_type_cast< ClassType*, false >(object)->*m_method)(
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
	typedef typename MethodSignature_6< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Const >::method_t method_t;

	method_t m_method;

	Method_6(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		ReturnType returnValue = (checked_type_cast< ClassType*, false >(object)->*m_method)(
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
	typedef typename MethodSignature_6< ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Const >::method_t method_t;

	method_t m_method;

	Method_6(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		(checked_type_cast< ClassType*, false >(object)->*m_method)(
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
	typedef typename MethodSignature_7< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Const >::method_t method_t;

	method_t m_method;

	Method_7(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		ReturnType returnValue = (checked_type_cast< ClassType*, false >(object)->*m_method)(
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
	typedef typename MethodSignature_7< ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Const >::method_t method_t;

	method_t m_method;

	Method_7(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		(checked_type_cast< ClassType*, false >(object)->*m_method)(
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
	typedef typename MethodSignature_8< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type, Const >::method_t method_t;

	method_t m_method;

	Method_8(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		ReturnType returnValue = (checked_type_cast< ClassType*, false >(object)->*m_method)(
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
	typedef typename MethodSignature_8< ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type, Const >::method_t method_t;

	method_t m_method;

	Method_8(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		(checked_type_cast< ClassType*, false >(object)->*m_method)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2]),
			CastAny< Argument4Type >::get(argv[3]),
			CastAny< Argument5Type >::get(argv[4]),
			CastAny< Argument6Type >::get(argv[5]),
			CastAny< Argument7Type >::get(argv[6]),
			CastAny< Argument7Type >::get(argv[7])
		);
		return Any();
	}
};

template <
	typename ClassType,
	typename ReturnType,
	bool Const
>
class Method_Variadic : public IMethod
{
	typedef typename MethodSignature_Variadic< ClassType, ReturnType, Const >::method_t method_t;

	method_t m_method;

	Method_Variadic(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		ReturnType returnValue = (checked_type_cast< ClassType*, false >(object)->*m_method)(argc, argv);
		return CastAny< ReturnType >::set(returnValue);
	}
};

template <
	typename ClassType,
	bool Const
>
class Method_Variadic < ClassType, void, Const > : public IMethod
{
	typedef typename MethodSignature_Variadic< ClassType, void, Const >::method_t method_t;

	method_t m_method;

	Method_Variadic(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		(checked_type_cast< ClassType*, false >(object)->*m_method)(argc, argv);
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
	typedef ReturnType (*method_t)(ClassType*);

	method_t m_method;

	MethodTrunk_0(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		ReturnType returnValue = (*m_method)(checked_type_cast< ClassType*, false >(object));
		return CastAny< ReturnType >::set(returnValue);
	}
};

template <
	typename ClassType
>
struct MethodTrunk_0< ClassType, void > : public IMethod
{
	typedef void (*method_t)(ClassType*);

	method_t m_method;

	MethodTrunk_0(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		(*m_method)(checked_type_cast< ClassType*, false >(object));
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
	typedef ReturnType (*method_t)(ClassType*, Argument1Type);

	method_t m_method;

	MethodTrunk_1(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		ReturnType returnValue = (*m_method)(
			checked_type_cast< ClassType*, false >(object),
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
	typedef void (*method_t)(ClassType*, Argument1Type);

	method_t m_method;

	MethodTrunk_1(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		(*m_method)(
			checked_type_cast< ClassType*, false >(object),
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
	typedef ReturnType (*method_t)(ClassType*, Argument1Type, Argument2Type);

	method_t m_method;

	MethodTrunk_2(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		ReturnType returnValue = (*m_method)(
			checked_type_cast< ClassType*, false >(object),
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
	typedef void (*method_t)(ClassType*, Argument1Type, Argument2Type);

	method_t m_method;

	MethodTrunk_2(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		(*m_method)(
			checked_type_cast< ClassType*, false >(object),
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
	typedef ReturnType (*method_t)(ClassType*, Argument1Type, Argument2Type, Argument3Type);

	method_t m_method;

	MethodTrunk_3(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		ReturnType returnValue = (*m_method)(
			checked_type_cast< ClassType*, false >(object),
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
	typedef void (*method_t)(ClassType*, Argument1Type, Argument2Type, Argument3Type);

	method_t m_method;

	MethodTrunk_3(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		(*m_method)(
			checked_type_cast< ClassType*, false >(object),
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
	typedef ReturnType (*method_t)(ClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type);

	method_t m_method;

	MethodTrunk_4(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		ReturnType returnValue = (*m_method)(
			checked_type_cast< ClassType*, false >(object),
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
	typedef void (*method_t)(ClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type);

	method_t m_method;

	MethodTrunk_4(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		(*m_method)(
			checked_type_cast< ClassType*, false >(object),
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
	typedef ReturnType (*method_t)(ClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type);

	method_t m_method;

	MethodTrunk_5(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		ReturnType returnValue = (*m_method)(
			checked_type_cast< ClassType*, false >(object),
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
	typedef ReturnType (*method_t)(ClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type);

	method_t m_method;

	MethodTrunk_6(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		ReturnType returnValue = (*m_method)(
			checked_type_cast< ClassType*, false >(object),
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
	typedef ReturnType (*method_t)(ClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type);

	method_t m_method;

	MethodTrunk_7(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		ReturnType returnValue = (*m_method)(
			checked_type_cast< ClassType*, false >(object),
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
	typedef ReturnType (*method_t)(ClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type);

	method_t m_method;

	MethodTrunk_8(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		ReturnType returnValue = (*m_method)(
			checked_type_cast< ClassType*, false >(object),
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
	typename ReturnType
>
struct MethodTrunk_Variadic : public IMethod
{
	typedef ReturnType (*method_t)(ClassType*, uint32_t, const Any*);

	method_t m_method;

	MethodTrunk_Variadic(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		ReturnType returnValue = (*m_method)(checked_type_cast< ClassType*, false >(object), argc, argv);
		return CastAny< ReturnType >::set(returnValue);
	}
};

template <
	typename ClassType
>
struct MethodTrunk_Variadic < ClassType, void > : public IMethod
{
	typedef void (*method_t)(ClassType*, uint32_t, const Any*);

	method_t m_method;

	MethodTrunk_Variadic(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, uint32_t argc, const Any* argv) const
	{
		(*m_method)(checked_type_cast< ClassType*, false >(object), argc, argv);
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
	typedef typename MethodSignature_0< ClassType, ReturnType, false >::static_method_t static_method_t;

	static_method_t m_method;

	StaticMethod_0(static_method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const
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
	typedef typename MethodSignature_0< ClassType, void, false >::static_method_t static_method_t;

	static_method_t m_method;

	StaticMethod_0(static_method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const
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
	typedef typename MethodSignature_1< ClassType, ReturnType, Argument1Type, false >::static_method_t static_method_t;

	static_method_t m_method;

	StaticMethod_1(static_method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const
	{
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
	typedef typename MethodSignature_1< ClassType, void, Argument1Type, false >::static_method_t static_method_t;

	static_method_t m_method;

	StaticMethod_1(static_method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const
	{
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
	typedef typename MethodSignature_2< ClassType, ReturnType, Argument1Type, Argument2Type, false >::static_method_t static_method_t;

	static_method_t m_method;

	StaticMethod_2(static_method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const
	{
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
	typedef typename MethodSignature_2< ClassType, void, Argument1Type, Argument2Type, false >::static_method_t static_method_t;

	static_method_t m_method;

	StaticMethod_2(static_method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const
	{
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
	typedef typename MethodSignature_3< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, false >::static_method_t static_method_t;

	static_method_t m_method;

	StaticMethod_3(static_method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const
	{
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
	typedef typename MethodSignature_3< ClassType, void, Argument1Type, Argument2Type, Argument3Type, false >::static_method_t static_method_t;

	static_method_t m_method;

	StaticMethod_3(static_method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const
	{
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
	typedef typename MethodSignature_4< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, false >::static_method_t static_method_t;

	static_method_t m_method;

	StaticMethod_4(static_method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const
	{
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
	typedef typename MethodSignature_4< ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, false >::static_method_t static_method_t;

	static_method_t m_method;

	StaticMethod_4(static_method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const
	{
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
	typedef typename MethodSignature_5< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, false >::static_method_t static_method_t;

	static_method_t m_method;

	StaticMethod_5(static_method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const
	{
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
	typedef typename MethodSignature_5< ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, false >::static_method_t static_method_t;

	static_method_t m_method;

	StaticMethod_5(static_method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const
	{
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
	typedef typename MethodSignature_6< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, false >::static_method_t static_method_t;

	static_method_t m_method;

	StaticMethod_6(static_method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const
	{
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
	typedef typename MethodSignature_6< ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, false >::static_method_t static_method_t;

	static_method_t m_method;

	StaticMethod_6(static_method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const
	{
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
	typedef typename MethodSignature_7< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, false >::static_method_t static_method_t;

	static_method_t m_method;

	StaticMethod_7(static_method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const
	{
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
	typedef typename MethodSignature_7< ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, false >::static_method_t static_method_t;

	static_method_t m_method;

	StaticMethod_7(static_method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const
	{
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

template <
	typename ClassType,
	typename ReturnType
>
class StaticMethod_Variadic : public IStaticMethod
{
	typedef ReturnType (*method_t)(uint32_t, const Any*);

	method_t m_method;

	StaticMethod_Variadic(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const
	{
		ReturnType returnValue = (*m_method)(argc, argv);
		return CastAny< ReturnType >::set(returnValue);
	}
};

template <
	typename ClassType
>
class StaticMethod_Variadic < ClassType, void > : public IStaticMethod
{
	typedef void (*method_t)(uint32_t, const Any*);

	method_t m_method;

	StaticMethod_Variadic(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(uint32_t argc, const Any* argv) const
	{
		(*m_method)(argc, argv);
		return Any();
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
	typedef typename MethodSignature_1< ClassType, ReturnType, Argument1Type, true >::method_t method_t;

	method_t m_method;

	Operator(method_t method)
	:	m_method(method)
	{
	}

	virtual bool tryPerform(Object* object, const Any& arg, Any& result) const
	{
		if (CastAny< Argument1Type >::accept(arg))
		{
			ClassType* target = checked_type_cast< ClassType*, false >(object);
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
 * \ingroup Script
 *
 * This class simplifies code necessary to map native classes
 * into script classes.
 * Just call addMethod with a pointer to your method and
 * it will automatically generate a "invoke" stub in compile
 * time.
 */
template < typename ClassType >
class AutoScriptClass : public IScriptClass
{
public:
	typedef Any (ClassType::*unknown_method_t)(const std::string& methodName, uint32_t argc, const Any* argv);

	AutoScriptClass()
	:	m_unknown(0)
	{
	}

	virtual ~AutoScriptClass()
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

	template <
		typename ReturnType
	>
	void addVariadicMethod(const std::string& methodName, ReturnType (ClassType::*method)(uint32_t, const Any*))
	{
		addVariadicMethod(methodName, new Method_Variadic< ClassType, ReturnType, false >(method));
	}

	template <
		typename ReturnType
	>
	void addVariadicMethod(const std::string& methodName, ReturnType (ClassType::*method)(uint32_t, const Any*) const)
	{
		addVariadicMethod(methodName, new Method_Variadic< ClassType, ReturnType, true >(method));
	}

	template <
		typename ReturnType
	>
	void addVariadicMethod(const std::string& methodName, ReturnType (*method)(ClassType*, uint32_t, const Any*))
	{
		addVariadicMethod(methodName, new MethodTrunk_Variadic< ClassType, ReturnType >(method));
	}

	void setUnknownMethod(unknown_method_t unknown)
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

	template <
		typename ReturnType
	>
	void addStaticVariadicMethod(const std::string& methodName, ReturnType (*method)(uint32_t, const Any*))
	{
		addVariadicMethod(methodName, new StaticMethod_Variadic< ClassType, ReturnType >(method));
	}

	/*! \} */

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

	virtual const TypeInfo& getExportType() const
	{
		return type_of< ClassType >();
	}

	virtual bool haveConstructor() const
	{
		return !m_constructors.empty();
	}

	virtual bool haveUnknown() const
	{
		return m_unknown != 0;
	}

	virtual Ref< Object > construct(const InvokeParam& param, uint32_t argc, const Any* argv) const
	{
		if (argc < m_constructors.size() && m_constructors[argc] != 0)
			return m_constructors[argc]->construct(argc, argv);
		else
			return 0;
	}

	virtual uint32_t getMethodCount() const
	{
		return uint32_t(m_methods.size());
	}

	virtual std::string getMethodName(uint32_t methodId) const
	{
		return m_methods[methodId].name;
	}

	virtual Any invoke(const InvokeParam& param, uint32_t methodId, uint32_t argc, const Any* argv) const
	{
		const MethodInfo& info = m_methods[methodId];
		const std::vector< IMethod* >& methods = info.methods;
		if (argc < methods.size() && methods[argc] != 0)
			return methods[argc]->invoke(param.object, argc, argv);
		else if (info.variadic)
			return info.variadic->invoke(param.object, argc, argv);
		else
			return Any();
	}

	virtual uint32_t getStaticMethodCount() const
	{
		return uint32_t(m_staticMethods.size());
	}

	virtual std::string getStaticMethodName(uint32_t methodId) const
	{
		return m_staticMethods[methodId].name;
	}

	virtual Any invokeStatic(uint32_t methodId, uint32_t argc, const Any* argv) const
	{
		const StaticMethodInfo& info = m_staticMethods[methodId];
		const std::vector< IStaticMethod* >& methods = info.methods;
		if (argc < methods.size() && methods[argc] != 0)
			return methods[argc]->invoke(argc, argv);
		else if (info.variadic)
			return info.variadic->invoke(argc, argv);
		else
			return Any();
	}

	virtual Any invokeUnknown(const InvokeParam& param, const std::string& methodName, uint32_t argc, const Any* argv) const
	{
		if (m_unknown)
			return (checked_type_cast< ClassType* >(param.object)->*m_unknown)(methodName, argc, argv);
		else
			return Any();
	}

	virtual Any invokeOperator(const InvokeParam& param, uint8_t operation, const Any& arg) const
	{
		Any result;

		// Try evaluate operation through handlers; first handler which return true
		// indicating successful evaluation aborts loop.
		const std::vector< IOperator* >& handlers = m_operators[operation];
		for (std::vector< IOperator* >::const_iterator i = handlers.begin(); i != handlers.end(); ++i)
		{
			if ((*i)->tryPerform(param.object, arg, result))
				break;
		}

		return result;
	}

private:
	struct MethodInfo
	{
		std::string name;
		std::vector< IMethod* > methods;
		IMethod* variadic;
	};

	struct StaticMethodInfo
	{
		std::string name;
		std::vector< IStaticMethod* > methods;
		IStaticMethod* variadic;
	};

	std::vector< IConstructor* > m_constructors;
	std::vector< MethodInfo > m_methods;
	std::vector< StaticMethodInfo > m_staticMethods;
	std::vector< IOperator* > m_operators[4];
	unknown_method_t m_unknown;

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
		m.variadic = 0;
		m_methods.push_back(m);
	}

	void addVariadicMethod(const std::string& methodName, IMethod* method)
	{
		for (typename std::vector< MethodInfo >::iterator i = m_methods.begin(); i != m_methods.end(); ++i)
		{
			if (i->name == methodName)
			{
				i->variadic = method;
				return;
			}
		}

		MethodInfo m;
		m.name = methodName;
		m.variadic = method;
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
		m.variadic = 0;
		m_staticMethods.push_back(m);
	}

	void addStaticVariadicMethod(const std::string& methodName, IStaticMethod* method)
	{
		for (typename std::vector< StaticMethodInfo >::iterator i = m_staticMethods.begin(); i != m_staticMethods.end(); ++i)
		{
			if (i->name == methodName)
			{
				i->variadic = method;
				return;
			}
		}

		StaticMethodInfo m;
		m.name = methodName;
		m.variadic = method;
		m_staticMethods.push_back(m);
	}
};

//@}

	}
}

#endif	// traktor_script_AutoScriptClass_H
