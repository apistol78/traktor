#ifndef traktor_script_AutoScriptClass_H
#define traktor_script_AutoScriptClass_H

#include "Script/ScriptClass.h"
#include "Core/Meta/TypeList.h"

namespace traktor
{
	namespace script
	{

//@{
/* \ingroup Script */


struct T_NOVTABLE IInvokable
{
	virtual Any invoke(Object* object, const std::vector< Any >& args) const = 0;
};


template <
	typename ClassType,
	typename ArgumentTypeList,
	bool ConstMethod,
	int ArgumentTypeCount = Length< ArgumentTypeList >::value
>
struct MethodSignature
{
};

template <
	typename ClassType,
	typename ArgumentTypeList
>
struct MethodSignature< ClassType, ArgumentTypeList, false, 1 >
{
	typedef typename Get< ArgumentTypeList, 0 >::type_t return_t;
	typedef return_t (ClassType::*method_t)();
};

template <
	typename ClassType,
	typename ArgumentTypeList
>
struct MethodSignature< ClassType, ArgumentTypeList, true, 1 >
{
	typedef typename Get< ArgumentTypeList, 0 >::type_t return_t;
	typedef return_t (ClassType::*method_t)() const;
};

template <
	typename ClassType,
	typename ArgumentTypeList
>
struct MethodSignature< ClassType, ArgumentTypeList, false, 2 >
{
	typedef typename Get< ArgumentTypeList, 0 >::type_t return_t;
	typedef typename Get< ArgumentTypeList, 1 >::type_t arg1_t;
	typedef return_t (ClassType::*method_t)(arg1_t);
};

template <
	typename ClassType,
	typename ArgumentTypeList
>
struct MethodSignature< ClassType, ArgumentTypeList, true, 2 >
{
	typedef typename Get< ArgumentTypeList, 0 >::type_t return_t;
	typedef typename Get< ArgumentTypeList, 1 >::type_t arg1_t;
	typedef return_t (ClassType::*method_t)(arg1_t) const;
};

template <
	typename ClassType,
	typename ArgumentTypeList
>
struct MethodSignature< ClassType, ArgumentTypeList, false, 3 >
{
	typedef typename Get< ArgumentTypeList, 0 >::type_t return_t;
	typedef typename Get< ArgumentTypeList, 1 >::type_t arg1_t;
	typedef typename Get< ArgumentTypeList, 2 >::type_t arg2_t;
	typedef return_t (ClassType::*method_t)(arg1_t, arg2_t);
};

template <
	typename ClassType,
	typename ArgumentTypeList
>
struct MethodSignature< ClassType, ArgumentTypeList, true, 3 >
{
	typedef typename Get< ArgumentTypeList, 0 >::type_t return_t;
	typedef typename Get< ArgumentTypeList, 1 >::type_t arg1_t;
	typedef typename Get< ArgumentTypeList, 2 >::type_t arg2_t;
	typedef return_t (ClassType::*method_t)(arg1_t, arg2_t) const;
};

template <
	typename ClassType,
	typename ArgumentTypeList
>
struct MethodSignature< ClassType, ArgumentTypeList, false, 4 >
{
	typedef typename Get< ArgumentTypeList, 0 >::type_t return_t;
	typedef typename Get< ArgumentTypeList, 1 >::type_t arg1_t;
	typedef typename Get< ArgumentTypeList, 2 >::type_t arg2_t;
	typedef typename Get< ArgumentTypeList, 3 >::type_t arg3_t;
	typedef return_t (ClassType::*method_t)(arg1_t, arg2_t, arg3_t);
};

template <
	typename ClassType,
	typename ArgumentTypeList
>
struct MethodSignature< ClassType, ArgumentTypeList, true, 4 >
{
	typedef typename Get< ArgumentTypeList, 0 >::type_t return_t;
	typedef typename Get< ArgumentTypeList, 1 >::type_t arg1_t;
	typedef typename Get< ArgumentTypeList, 2 >::type_t arg2_t;
	typedef typename Get< ArgumentTypeList, 3 >::type_t arg3_t;
	typedef return_t (ClassType::*method_t)(arg1_t, arg2_t, arg3_t) const;
};

template <
typename ClassType,
typename ArgumentTypeList
>
struct MethodSignature< ClassType, ArgumentTypeList, false, 5 >
{
	typedef typename Get< ArgumentTypeList, 0 >::type_t return_t;
	typedef typename Get< ArgumentTypeList, 1 >::type_t arg1_t;
	typedef typename Get< ArgumentTypeList, 2 >::type_t arg2_t;
	typedef typename Get< ArgumentTypeList, 3 >::type_t arg3_t;
	typedef typename Get< ArgumentTypeList, 4 >::type_t arg4_t;
	typedef return_t (ClassType::*method_t)(arg1_t, arg2_t, arg3_t, arg4_t);
};

template <
typename ClassType,
typename ArgumentTypeList
>
struct MethodSignature< ClassType, ArgumentTypeList, true, 5 >
{
	typedef typename Get< ArgumentTypeList, 0 >::type_t return_t;
	typedef typename Get< ArgumentTypeList, 1 >::type_t arg1_t;
	typedef typename Get< ArgumentTypeList, 2 >::type_t arg2_t;
	typedef typename Get< ArgumentTypeList, 3 >::type_t arg3_t;
	typedef typename Get< ArgumentTypeList, 4 >::type_t arg4_t;
	typedef return_t (ClassType::*method_t)(arg1_t, arg2_t, arg3_t, arg4_t) const;
};


template < typename Type, bool IsTypePtr = IsPointer< Type >::value >
struct CastAny { };

template < >
struct CastAny < bool, false > { static bool get(const Any& value) { return value.getBoolean(); } };

template < >
struct CastAny < int32_t, false > { static int32_t get(const Any& value) { return value.getInteger(); } };

template < >
struct CastAny < float, false > { static float get(const Any& value) { return value.getFloat(); } };

template < >
struct CastAny < std::wstring, false > { static std::wstring get(const Any& value) { return value.getString(); } };

template < >
struct CastAny < const std::wstring&, false > { static std::wstring get(const Any& value) { return value.getString(); } };

template < >
struct CastAny < const wchar_t, true > { static const wchar_t* get(const Any& value) { return value.getString().c_str(); } };

template < typename Type >
struct CastAny < Type, true > { static Type get(const Any& value) { return checked_type_cast< Type >(value.getObject()); } };


template <
	typename ClassType,
	typename ArgumentTypeList,
	bool ConstMethod,
	typename ReturnType = typename Get< ArgumentTypeList, 0 >::type_t,
	int ArgumentTypeCount = Length< ArgumentTypeList >::value
>
struct Invokable : public IInvokable
{
};


template <
	typename ClassType,
	typename ArgumentTypeList,
	bool ConstMethod,
	typename ReturnType
>
struct Invokable < ClassType, ArgumentTypeList, ConstMethod, ReturnType, 1 > : public IInvokable
{
	typedef typename MethodSignature< ClassType, ArgumentTypeList, ConstMethod >::method_t method_t;

	method_t m_method;

	Invokable(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, const std::vector< Any >& args) const
	{
		ReturnType returnValue = (checked_type_cast< ClassType* >(object)->*m_method)();
		return Any(returnValue);
	}
};

template <
	typename ClassType,
	typename ArgumentTypeList,
	bool ConstMethod
>
struct Invokable < ClassType, ArgumentTypeList, ConstMethod, void, 1 > : public IInvokable
{
	typedef typename MethodSignature< ClassType, ArgumentTypeList, ConstMethod >::method_t method_t;

	method_t m_method;

	Invokable(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, const std::vector< Any >& args) const
	{
		(checked_type_cast< ClassType* >(object)->*m_method)();
		return Any();
	}
};


template <
	typename ClassType,
	typename ArgumentTypeList,
	bool ConstMethod,
	typename ReturnType
>
struct Invokable < ClassType, ArgumentTypeList, ConstMethod, ReturnType, 2 > : public IInvokable
{
	typedef typename MethodSignature< ClassType, ArgumentTypeList, ConstMethod >::arg1_t arg1_t;
	typedef typename MethodSignature< ClassType, ArgumentTypeList, ConstMethod >::method_t method_t;

	method_t m_method;

	Invokable(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, const std::vector< Any >& args) const
	{
		T_ASSERT_M (args.size() >= 1, L"Not enough arguments");
		ReturnType returnValue = (checked_type_cast< ClassType* >(object)->*m_method)(
			CastAny< arg1_t >::get(args[0])
		);
		return Any(returnValue);
	}
};

template <
	typename ClassType,
	typename ArgumentTypeList,
	bool ConstMethod
>
struct Invokable < ClassType, ArgumentTypeList, ConstMethod, void, 2 > : public IInvokable
{
	typedef typename MethodSignature< ClassType, ArgumentTypeList, ConstMethod >::arg1_t arg1_t;
	typedef typename MethodSignature< ClassType, ArgumentTypeList, ConstMethod >::method_t method_t;

	method_t m_method;

	Invokable(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, const std::vector< Any >& args) const
	{
		T_ASSERT_M (args.size() >= 1, L"Not enough arguments");
		(checked_type_cast< ClassType* >(object)->*m_method)(
			CastAny< arg1_t >::get(args[0])
		);
		return Any();
	}
};


template <
	typename ClassType,
	typename ArgumentTypeList,
	bool ConstMethod,
	typename ReturnType
>
struct Invokable < ClassType, ArgumentTypeList, ConstMethod, ReturnType, 3 > : public IInvokable
{
	typedef typename MethodSignature< ClassType, ArgumentTypeList, ConstMethod >::arg1_t arg1_t;
	typedef typename MethodSignature< ClassType, ArgumentTypeList, ConstMethod >::arg2_t arg2_t;
	typedef typename MethodSignature< ClassType, ArgumentTypeList, ConstMethod >::method_t method_t;

	method_t m_method;

	Invokable(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, const std::vector< Any >& args) const
	{
		T_ASSERT_M (args.size() >= 2, L"Not enough arguments");
		ReturnType returnValue = (checked_type_cast< ClassType* >(object)->*m_method)(
			CastAny< arg1_t >::get(args[0]),
			CastAny< arg2_t >::get(args[1])
		);
		return Any(returnValue);
	}
};

template <
	typename ClassType,
	typename ArgumentTypeList,
	bool ConstMethod
>
struct Invokable < ClassType, ArgumentTypeList, ConstMethod, void, 3 > : public IInvokable
{
	typedef typename MethodSignature< ClassType, ArgumentTypeList, ConstMethod >::arg1_t arg1_t;
	typedef typename MethodSignature< ClassType, ArgumentTypeList, ConstMethod >::arg2_t arg2_t;
	typedef typename MethodSignature< ClassType, ArgumentTypeList, ConstMethod >::method_t method_t;

	method_t m_method;

	Invokable(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, const std::vector< Any >& args) const
	{
		T_ASSERT_M (args.size() >= 2, L"Not enough arguments");
		(checked_type_cast< ClassType* >(object)->*m_method)(
			CastAny< arg1_t >::get(args[0]),
			CastAny< arg2_t >::get(args[1])
		);
		return Any();
	}
};


template <
	typename ClassType,
	typename ArgumentTypeList,
	bool ConstMethod,
	typename ReturnType
>
struct Invokable < ClassType, ArgumentTypeList, ConstMethod, ReturnType, 4 > : public IInvokable
{
	typedef typename MethodSignature< ClassType, ArgumentTypeList, ConstMethod >::arg1_t arg1_t;
	typedef typename MethodSignature< ClassType, ArgumentTypeList, ConstMethod >::arg2_t arg2_t;
	typedef typename MethodSignature< ClassType, ArgumentTypeList, ConstMethod >::arg3_t arg3_t;
	typedef typename MethodSignature< ClassType, ArgumentTypeList, ConstMethod >::method_t method_t;

	method_t m_method;

	Invokable(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, const std::vector< Any >& args) const
	{
		T_ASSERT_M (args.size() >= 3, L"Not enough arguments");
		ReturnType returnValue = (checked_type_cast< ClassType* >(object)->*m_method)(
			CastAny< arg1_t >::get(args[0]),
			CastAny< arg2_t >::get(args[1]),
			CastAny< arg3_t >::get(args[2])
		);
		return Any(returnValue);
	}
};

template <
	typename ClassType,
	typename ArgumentTypeList,
	bool ConstMethod
>
struct Invokable < ClassType, ArgumentTypeList, ConstMethod, void, 4 > : public IInvokable
{
	typedef typename MethodSignature< ClassType, ArgumentTypeList, ConstMethod >::arg1_t arg1_t;
	typedef typename MethodSignature< ClassType, ArgumentTypeList, ConstMethod >::arg2_t arg2_t;
	typedef typename MethodSignature< ClassType, ArgumentTypeList, ConstMethod >::arg3_t arg3_t;
	typedef typename MethodSignature< ClassType, ArgumentTypeList, ConstMethod >::method_t method_t;

	method_t m_method;

	Invokable(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, const std::vector< Any >& args) const
	{
		T_ASSERT_M (args.size() >= 3, L"Not enough arguments");
		(checked_type_cast< ClassType* >(object)->*m_method)(
			CastAny< arg1_t >::get(args[0]),
			CastAny< arg2_t >::get(args[1]),
			CastAny< arg3_t >::get(args[2])
		);
		return Any();
	}
};


template <
	typename ClassType,
	typename ArgumentTypeList,
	bool ConstMethod,
	typename ReturnType
>
struct Invokable < ClassType, ArgumentTypeList, ConstMethod, ReturnType, 5 > : public IInvokable
{
	typedef typename MethodSignature< ClassType, ArgumentTypeList, ConstMethod >::arg1_t arg1_t;
	typedef typename MethodSignature< ClassType, ArgumentTypeList, ConstMethod >::arg2_t arg2_t;
	typedef typename MethodSignature< ClassType, ArgumentTypeList, ConstMethod >::arg3_t arg3_t;
	typedef typename MethodSignature< ClassType, ArgumentTypeList, ConstMethod >::arg4_t arg4_t;
	typedef typename MethodSignature< ClassType, ArgumentTypeList, ConstMethod >::method_t method_t;

	method_t m_method;

	Invokable(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, const std::vector< Any >& args) const
	{
		T_ASSERT_M (args.size() >= 4, L"Not enough arguments");
		ReturnType returnValue = (checked_type_cast< ClassType* >(object)->*m_method)(
			CastAny< arg1_t >::get(args[0]),
			CastAny< arg2_t >::get(args[1]),
			CastAny< arg3_t >::get(args[2]),
			CastAny< arg4_t >::get(args[3]),
		);
		return Any(returnValue);
	}
};

template <
	typename ClassType,
	typename ArgumentTypeList,
	bool ConstMethod
>
struct Invokable < ClassType, ArgumentTypeList, ConstMethod, void, 5 > : public IInvokable
{
	typedef typename MethodSignature< ClassType, ArgumentTypeList, ConstMethod >::arg1_t arg1_t;
	typedef typename MethodSignature< ClassType, ArgumentTypeList, ConstMethod >::arg2_t arg2_t;
	typedef typename MethodSignature< ClassType, ArgumentTypeList, ConstMethod >::arg3_t arg3_t;
	typedef typename MethodSignature< ClassType, ArgumentTypeList, ConstMethod >::arg4_t arg4_t;
	typedef typename MethodSignature< ClassType, ArgumentTypeList, ConstMethod >::method_t method_t;

	method_t m_method;

	Invokable(method_t method)
	:	m_method(method)
	{
	}

	virtual Any invoke(Object* object, const std::vector< Any >& args) const
	{
		T_ASSERT_M (args.size() >= 4, L"Not enough arguments");
		(checked_type_cast< ClassType* >(object)->*m_method)(
			CastAny< arg1_t >::get(args[0]),
			CastAny< arg2_t >::get(args[1]),
			CastAny< arg3_t >::get(args[2]),
			CastAny< arg4_t >::get(args[3])
		);
		return Any();
	}
};


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
class AutoScriptClass : public ScriptClass
{
public:
	AutoScriptClass()
	{
	}

	virtual ~AutoScriptClass()
	{
		T_EXCEPTION_GUARD_BEGIN
		for (std::vector< MethodInfo >::iterator i = m_methods.begin(); i != m_methods.end(); ++i)
			delete i->mptr;
		T_EXCEPTION_GUARD_END
	}

	template < typename ArgumentTypeList >
	void addMethod(const std::wstring& methodName, typename Invokable< ClassType, ArgumentTypeList, false >::method_t method)
	{
		MethodInfo info = { methodName, new Invokable< ClassType, ArgumentTypeList, false >(method) };
		m_methods.push_back(info);
	}

	template < typename ArgumentTypeList >
	void addConstMethod(const std::wstring& methodName, typename Invokable< ClassType, ArgumentTypeList, true >::method_t method)
	{
		MethodInfo info = { methodName, new Invokable< ClassType, ArgumentTypeList, true >(method) };
		m_methods.push_back(info);
	}

	virtual const Type& getExportType() const
	{
		return type_of< ClassType >();
	}

	virtual uint32_t getMethodCount() const
	{
		return uint32_t(m_methods.size());
	}

	virtual std::wstring getMethodName(uint32_t methodId) const
	{
		return m_methods[methodId].name;
	}

	virtual Any invoke(Object* object, uint32_t methodId, const std::vector< Any >& args) const
	{
		return m_methods[methodId].mptr->invoke(object, args);
	}

private:
	struct MethodInfo
	{
		std::wstring name;
		IInvokable* mptr;
	};

	std::vector< MethodInfo > m_methods;
};

//@}

	}
}

#endif	// traktor_script_AutoScriptClass_H
