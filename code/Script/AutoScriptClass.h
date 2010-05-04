#ifndef traktor_script_AutoScriptClass_H
#define traktor_script_AutoScriptClass_H

#include "Script/IScriptClass.h"
#include "Core/Meta/TypeList.h"

namespace traktor
{
	namespace script
	{

/*! \ingroup Script */
/*! \{ */


struct T_NOVTABLE IConstructor
{
	virtual Ref< Object > construct(const Any* argv) const = 0;
};

struct T_NOVTABLE IMethod
{
	virtual Any invoke(Object* object, const Any* argv) const = 0;
};

struct T_NOVTABLE IProperty
{
	virtual Any get(const Object* object) const = 0;

	virtual void set(Object* object, const Any& value) const = 0;
};


/*! \name Any cast */
/*! \{ */

template < typename Type, bool IsTypePtr = IsPointer< Type >::value >
struct CastAny { };

template < >
struct CastAny < bool, false > { static bool get(const Any& value) { return value.getBoolean(); } };

template < >
struct CastAny < int32_t, false > { static int32_t get(const Any& value) { return value.getInteger(); } };

template < >
struct CastAny < uint32_t, false > { static uint32_t get(const Any& value) { return (uint32_t)value.getInteger(); } };

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

/*! \} */


/*! \name Constructors */
/*! \{ */

template<
	typename ClassType
>
struct Constructor_0 : public IConstructor
{
	virtual Ref< Object > construct(const Any* argv) const
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
	virtual Ref< Object > construct(const Any* argv) const
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
	virtual Ref< Object > construct(const Any* argv) const
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
	virtual Ref< Object > construct(const Any* argv) const
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
	virtual Ref< Object > construct(const Any* argv) const
	{
		return new ClassType(
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
	typedef ReturnType (ClassType::*method_t)();
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

	virtual Any invoke(Object* object, const Any* argv) const
	{
		ReturnType returnValue = (checked_type_cast< ClassType*, false >(object)->*m_method)();
		return Any(returnValue);
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

	virtual Any invoke(Object* object, const Any* argv) const
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

	virtual Any invoke(Object* object, const Any* argv) const
	{
		ReturnType returnValue = (checked_type_cast< ClassType*, false >(object)->*m_method)(
			CastAny< Argument1Type >::get(argv[0])
		);
		return Any(returnValue);
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

	virtual Any invoke(Object* object, const Any* argv) const
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

	virtual Any invoke(Object* object, const Any* argv) const
	{
		ReturnType returnValue = (checked_type_cast< ClassType*, false >(object)->*m_method)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1])
		);
		return Any(returnValue);
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

	virtual Any invoke(Object* object, const Any* argv) const
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

	virtual Any invoke(Object* object, const Any* argv) const
	{
		ReturnType returnValue = (checked_type_cast< ClassType*, false >(object)->*m_method)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2])
		);
		return Any(returnValue);
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

	virtual Any invoke(Object* object, const Any* argv) const
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

	virtual Any invoke(Object* object, const Any* argv) const
	{
		ReturnType returnValue = (checked_type_cast< ClassType*, false >(object)->*m_method)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2]),
			CastAny< Argument4Type >::get(argv[3])
		);
		return Any(returnValue);
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

	virtual Any invoke(Object* object, const Any* argv) const
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

	virtual Any invoke(Object* object, const Any* argv) const
	{
		ReturnType returnValue = (checked_type_cast< ClassType*, false >(object)->*m_method)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2]),
			CastAny< Argument4Type >::get(argv[3]),
			CastAny< Argument5Type >::get(argv[4])
		);
		return Any(returnValue);
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

	virtual Any invoke(Object* object, const Any* argv) const
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

/*! \} */


/*! \name Property accessor */
/*! \{ */

template <
	typename ClassType,
	typename MemberType
>
struct Property : public IProperty
{
	typedef MemberType ClassType::*member_t;

	member_t m_member;

	Property(member_t member)
	:	m_member(member)
	{
	}

	virtual Any get(const Object* object) const
	{
		return Any(checked_type_cast< const ClassType*, false >(object)->*m_member);
	}

	virtual void set(Object* object, const Any& value) const
	{
		checked_type_cast< ClassType*, false >(object)->*m_member = CastAny< MemberType >::get(value);
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
	typedef Any (ClassType::*unknown_method_t)(const std::wstring& methodName, uint32_t argc, const Any* argv);

	AutoScriptClass()
	{
	}

	virtual ~AutoScriptClass()
	{
		T_EXCEPTION_GUARD_BEGIN
		
		for (std::vector< IConstructor* >::iterator i = m_constructors.begin(); i != m_constructors.end(); ++i)
			delete *i;
		
		for (std::vector< std::pair< std::wstring, std::vector< IMethod* > > >::iterator i = m_methods.begin(); i != m_methods.end(); ++i)
		{
			for (std::vector< IMethod* >::iterator j = i->second.begin(); j != i->second.end(); ++j)
				delete *j;
		}
		
		for (std::vector< std::pair< std::wstring, IProperty* > >::iterator i = m_properties.begin(); i != m_properties.end(); ++i)
			delete i->second;

		T_EXCEPTION_GUARD_END
	}

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
		typename ReturnType
	>
	void addMethod(const std::wstring& methodName, ReturnType (ClassType::*method)())
	{
		addMethod(methodName, 0, new Method_0< ClassType, ReturnType, false >(method));
	}

	template <
		typename ReturnType
	>
	void addMethod(const std::wstring& methodName, ReturnType (ClassType::*method)() const)
	{
		addMethod(methodName, 0, new Method_0< ClassType, ReturnType, true >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type
	>
	void addMethod(const std::wstring& methodName, ReturnType (ClassType::*method)(Argument1Type))
	{
		addMethod(methodName, 1, new Method_1< ClassType, ReturnType, Argument1Type, false >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type
	>
	void addMethod(const std::wstring& methodName, ReturnType (ClassType::*method)(Argument1Type) const)
	{
		addMethod(methodName, 1, new Method_1< ClassType, ReturnType, Argument1Type, true >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type
	>
	void addMethod(const std::wstring& methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type))
	{
		addMethod(methodName, 2, new Method_2< ClassType, ReturnType, Argument1Type, Argument2Type, false >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type
	>
	void addMethod(const std::wstring& methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type) const)
	{
		addMethod(methodName, 2, new Method_2< ClassType, ReturnType, Argument1Type, Argument2Type, true >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type
	>
	void addMethod(const std::wstring& methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type))
	{
		addMethod(methodName, 3, new Method_3< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, false >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type
	>
	void addMethod(const std::wstring& methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type) const)
	{
		addMethod(methodName, 3, new Method_3< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, true >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type
	>
	void addMethod(const std::wstring& methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type))
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
	void addMethod(const std::wstring& methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type) const)
	{
		addMethod(methodName, 4, new Method_4< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, true >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type,
		typename Argument5Type
	>
	void addMethod(const std::wstring& methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type))
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
	void addMethod(const std::wstring& methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type) const)
	{
		addMethod(methodName, 5, new Method_5< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, true >(method));
	}

	void setUnknownMethod(unknown_method_t unknown)
	{
		m_unknown = unknown;
	}

	template <
		typename MemberType
	>
	void addProperty(const std::wstring& memberName, MemberType ClassType::*member)
	{
		m_properties.push_back(std::make_pair(
			memberName,
			new Property< ClassType, MemberType >(member)
		));
	}

	virtual const TypeInfo& getExportType() const
	{
		return type_of< ClassType >();
	}

	virtual bool haveConstructor() const
	{
		return !m_constructors.empty();
	}

	virtual Ref< Object > construct(uint32_t argc, const Any* argv) const
	{
		if (argc < m_constructors.size() && m_constructors[argc] != 0)
			return m_constructors[argc]->construct(argv);
		else
			return 0;
	}

	virtual uint32_t getMethodCount() const
	{
		return uint32_t(m_methods.size());
	}

	virtual std::wstring getMethodName(uint32_t methodId) const
	{
		return m_methods[methodId].first;
	}

	virtual Any invoke(Object* object, uint32_t methodId, uint32_t argc, const Any* argv) const
	{
		const std::vector< IMethod* >& methods = m_methods[methodId].second;
		if (argc < methods.size() && methods[argc] != 0)
			return methods[argc]->invoke(object, argv);
		else
			return Any();
	}

	virtual Any invokeUnknown(Object* object, const std::wstring& methodName, uint32_t argc, const Any* argv) const
	{
		if (m_unknown)
			return (checked_type_cast< ClassType* >(object)->*m_unknown)(methodName, argc, argv);
		else
			return Any();
	}

	virtual uint32_t getPropertyCount() const
	{
		return uint32_t(m_properties.size());
	}

	virtual std::wstring getPropertyName(uint32_t propertyId) const
	{
		return m_properties[propertyId].first;
	}

	virtual Any getPropertyValue(const Object* object, uint32_t propertyId) const
	{
		return m_properties[propertyId].second->get(object);
	}

	virtual void setPropertyValue(Object* object, uint32_t propertyId, const Any& value) const
	{
		m_properties[propertyId].second->set(object, value);
	}

private:
	std::vector< IConstructor* > m_constructors;
	std::vector< std::pair< std::wstring, std::vector< IMethod* > > > m_methods;
	std::vector< std::pair< std::wstring, IProperty* > > m_properties;
	unknown_method_t m_unknown;

	void addConstructor(size_t argc, IConstructor* constructor)
	{
		if (m_constructors.size() < argc + 1)
			m_constructors.resize(argc + 1, 0);
		m_constructors[argc] = constructor;
	}

	void addMethod(const std::wstring& methodName, size_t argc, IMethod* method)
	{
		for (std::vector< std::pair< std::wstring, std::vector< IMethod* > > >::iterator i = m_methods.begin(); i != m_methods.end(); ++i)
		{
			if (i->first == methodName)
			{
				std::vector< IMethod* >& methods = i->second;
				if (methods.size() < argc + 1)
					methods.resize(argc + 1, 0);
				methods[argc] = method;
				return;
			}
		}

		std::pair< std::wstring, std::vector< IMethod* > > m;
		m.first = methodName;
		m.second.resize(argc + 1, 0);
		m.second[argc] = method;
		m_methods.push_back(m);
	}
};

//@}

	}
}

#endif	// traktor_script_AutoScriptClass_H
