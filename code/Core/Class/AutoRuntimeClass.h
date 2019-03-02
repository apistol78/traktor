#pragma once

#include "Core/Class/AutoConstructor.h"
#include "Core/Class/AutoMethod.h"
#include "Core/Class/AutoMethodTrunk.h"
#include "Core/Class/AutoOperator.h"
#include "Core/Class/AutoProperty.h"
#include "Core/Class/AutoStaticMethod.h"
#include "Core/Class/AutoUnknown.h"
#include "Core/Class/OperatorDispatch.h"
#include "Core/Class/RuntimeClass.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/TString.h"

namespace traktor
{

/*! \ingroup Core */
/*! \{ */

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
class AutoRuntimeClass : public RuntimeClass
{
public:
	T_NO_COPY_CLASS(AutoRuntimeClass);

	AutoRuntimeClass()
	{
	}

	void setUnknownHandler(typename Unknown< ClassType >::unknown_fn_t unknown)
	{
		m_unknown = new Unknown< ClassType >(unknown);
	}

	/*! \name Constructors */
	/*! \{ */

	void addConstructor()
	{
		RuntimeClass::addConstructor(0, new Constructor_0< ClassType >());
	}

	template <
		typename Argument1Type
	>
	void addConstructor()
	{
		RuntimeClass::addConstructor(1, new Constructor_1< ClassType, Argument1Type >());
	}

	template <
		typename Argument1Type,
		typename Argument2Type
	>
	void addConstructor()
	{
		RuntimeClass::addConstructor(2, new Constructor_2< ClassType, Argument1Type, Argument2Type >());
	}

	template <
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type
	>
	void addConstructor()
	{
		RuntimeClass::addConstructor(3, new Constructor_3< ClassType, Argument1Type, Argument2Type, Argument3Type >());
	}

	template <
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type
	>
	void addConstructor()
	{
		RuntimeClass::addConstructor(4, new Constructor_4< ClassType, Argument1Type, Argument2Type, Argument3Type, Argument4Type >());
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
		RuntimeClass::addConstructor(5, new Constructor_5< ClassType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type >());
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
		RuntimeClass::addConstructor(6, new Constructor_6< ClassType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type >());
	}

	template <
		typename Argument1Type
	>
	void addConstructor(typename FnConstructor_1< ClassType, Argument1Type >::fn_t fn)
	{
		RuntimeClass::addConstructor(1, new FnConstructor_1< ClassType, Argument1Type >(fn));
	}

	template <
		typename Argument1Type,
		typename Argument2Type
	>
	void addConstructor(typename FnConstructor_2< ClassType, Argument1Type, Argument2Type >::fn_t fn)
	{
		RuntimeClass::addConstructor(2, new FnConstructor_2< ClassType, Argument1Type, Argument2Type >(fn));
	}

	template <
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type
	>
	void addConstructor(typename FnConstructor_3< ClassType, Argument1Type, Argument2Type, Argument3Type >::fn_t fn)
	{
		RuntimeClass::addConstructor(3, new FnConstructor_3< ClassType, Argument1Type, Argument2Type, Argument3Type >(fn));
	}

	template <
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type
	>
	void addConstructor(typename FnConstructor_4< ClassType, Argument1Type, Argument2Type, Argument3Type, Argument4Type >::fn_t fn)
	{
		RuntimeClass::addConstructor(4, new FnConstructor_4< ClassType, Argument1Type, Argument2Type, Argument3Type, Argument4Type >(fn));
	}

	/*! \} */

	/*! \name Methods */
	/*! \{ */

	template <
		typename ReturnType
	>
	void addMethod(const char* const methodName, ReturnType (ClassType::*method)())
	{
		RuntimeClass::addMethod(methodName, 0, new Method_0< ClassType, ReturnType, false >(method));
	}

	template <
		typename ReturnType
	>
	void addMethod(const char* const methodName, ReturnType (ClassType::*method)() const)
	{
		RuntimeClass::addMethod(methodName, 0, new Method_0< ClassType, ReturnType, true >(method));
	}

	template <
		typename ReturnType
	>
	void addMethod(const char* const methodName, ReturnType (*method)(ClassType*))
	{
		RuntimeClass::addMethod(methodName, 0, new MethodTrunk_0< ClassType, ReturnType >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type
	>
	void addMethod(const char* const methodName, ReturnType (ClassType::*method)(Argument1Type))
	{
		RuntimeClass::addMethod(methodName, 1, new Method_1< ClassType, ReturnType, Argument1Type, false >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type
	>
	void addMethod(const char* const methodName, ReturnType (ClassType::*method)(Argument1Type) const)
	{
		RuntimeClass::addMethod(methodName, 1, new Method_1< ClassType, ReturnType, Argument1Type, true >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type
	>
	void addMethod(const char* const methodName, ReturnType (*method)(ClassType*, Argument1Type))
	{
		RuntimeClass::addMethod(methodName, 1, new MethodTrunk_1< ClassType, ReturnType, Argument1Type >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type
	>
	void addMethod(const char* const methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type))
	{
		RuntimeClass::addMethod(methodName, 2, new Method_2< ClassType, ReturnType, Argument1Type, Argument2Type, false >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type
	>
	void addMethod(const char* const methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type) const)
	{
		RuntimeClass::addMethod(methodName, 2, new Method_2< ClassType, ReturnType, Argument1Type, Argument2Type, true >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type
	>
	void addMethod(const char* const methodName, ReturnType (*method)(ClassType*, Argument1Type, Argument2Type))
	{
		RuntimeClass::addMethod(methodName, 2, new MethodTrunk_2< ClassType, ReturnType, Argument1Type, Argument2Type >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type
	>
	void addMethod(const char* const methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type))
	{
		RuntimeClass::addMethod(methodName, 3, new Method_3< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, false >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type
	>
	void addMethod(const char* const methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type) const)
	{
		RuntimeClass::addMethod(methodName, 3, new Method_3< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, true >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type
	>
	void addMethod(const char* const methodName, ReturnType (*method)(ClassType*, Argument1Type, Argument2Type, Argument3Type))
	{
		RuntimeClass::addMethod(methodName, 3, new MethodTrunk_3< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type
	>
	void addMethod(const char* const methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type))
	{
		RuntimeClass::addMethod(methodName, 4, new Method_4< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, false >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type
	>
	void addMethod(const char* const methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type) const)
	{
		RuntimeClass::addMethod(methodName, 4, new Method_4< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, true >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type
	>
	void addMethod(const char* const methodName, ReturnType (*method)(ClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type))
	{
		RuntimeClass::addMethod(methodName, 4, new MethodTrunk_4< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type,
		typename Argument5Type
	>
	void addMethod(const char* const methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type))
	{
		RuntimeClass::addMethod(methodName, 5, new Method_5< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, false >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type,
		typename Argument5Type
	>
	void addMethod(const char* const methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type) const)
	{
		RuntimeClass::addMethod(methodName, 5, new Method_5< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, true >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type,
		typename Argument5Type
	>
	void addMethod(const char* const methodName, ReturnType (*method)(ClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type))
	{
		RuntimeClass::addMethod(methodName, 5, new MethodTrunk_5< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type >(method));
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
	void addMethod(const char* const methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type))
	{
		RuntimeClass::addMethod(methodName, 6, new Method_6< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, false >(method));
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
	void addMethod(const char* const methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type) const)
	{
		RuntimeClass::addMethod(methodName, 6, new Method_6< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, true >(method));
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
	void addMethod(const char* const methodName, ReturnType (*method)(ClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type))
	{
		RuntimeClass::addMethod(methodName, 6, new MethodTrunk_6< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type >(method));
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
	void addMethod(const char* const methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type))
	{
		RuntimeClass::addMethod(methodName, 7, new Method_7< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, false >(method));
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
	void addMethod(const char* const methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type) const)
	{
		RuntimeClass::addMethod(methodName, 7, new Method_7< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, true >(method));
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
	void addMethod(const char* const methodName, ReturnType (*method)(ClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type))
	{
		RuntimeClass::addMethod(methodName, 7, new MethodTrunk_7< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type >(method));
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
	void addMethod(const char* const methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type))
	{
		RuntimeClass::addMethod(methodName, 8, new Method_8< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type, false >(method));
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
	void addMethod(const char* const methodName, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type) const)
	{
		RuntimeClass::addMethod(methodName, 8, new Method_8< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type, true >(method));
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
	void addMethod(const char* const methodName, ReturnType (*method)(ClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type))
	{
		RuntimeClass::addMethod(methodName, 8, new MethodTrunk_8< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type >(method));
	}

	/*! \} */

	/*! \name Static methods */
	/*! \{ */

	template <
		typename ReturnType
	>
	void addStaticMethod(const char* const methodName, ReturnType (*method)())
	{
		RuntimeClass::addStaticMethod(methodName, 0, new StaticMethod_0< ClassType, ReturnType >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type
	>
	void addStaticMethod(const char* const methodName, ReturnType (*method)(Argument1Type))
	{
		RuntimeClass::addStaticMethod(methodName, 1, new StaticMethod_1< ClassType, ReturnType, Argument1Type >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type
	>
	void addStaticMethod(const char* const methodName, ReturnType (*method)(Argument1Type, Argument2Type))
	{
		RuntimeClass::addStaticMethod(methodName, 2, new StaticMethod_2< ClassType, ReturnType, Argument1Type, Argument2Type >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type
	>
	void addStaticMethod(const char* const methodName, ReturnType (*method)(Argument1Type, Argument2Type, Argument3Type))
	{
		RuntimeClass::addStaticMethod(methodName, 3, new StaticMethod_3< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type
	>
	void addStaticMethod(const char* const methodName, ReturnType (*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type))
	{
		RuntimeClass::addStaticMethod(methodName, 4, new StaticMethod_4< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type >(method));
	}

	template <
		typename ReturnType,
		typename Argument1Type,
		typename Argument2Type,
		typename Argument3Type,
		typename Argument4Type,
		typename Argument5Type
	>
	void addStaticMethod(const char* const methodName, ReturnType (*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type))
	{
		RuntimeClass::addStaticMethod(methodName, 5, new StaticMethod_5< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type >(method));
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
	void addStaticMethod(const char* const methodName, ReturnType (*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type))
	{
		RuntimeClass::addStaticMethod(methodName, 6, new StaticMethod_6< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type >(method));
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
	void addStaticMethod(const char* const methodName, ReturnType (*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type))
	{
		RuntimeClass::addStaticMethod(methodName, 7, new StaticMethod_7< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type >(method));
	}

	/*! \} */

	template < typename ValueType >
	void addProperty(const char* const propertyName, ValueType (ClassType::*getter)() const)
	{
		StringOutputStream ss;
		CastAny< ValueType >::typeName(ss);

		RuntimeClass::addProperty(
			propertyName,
			ss.str(),
			0,
			getter != 0 ? new PropertyGet< ClassType, ValueType, true >(getter) : 0
		);
	}

	template < typename SetterValueType, typename GetterValueType >
	void addProperty(const char* const propertyName, void (ClassType::*setter)(SetterValueType value), GetterValueType (ClassType::*getter)() const)
	{
		StringOutputStream ss;
		CastAny< GetterValueType >::typeName(ss);

		RuntimeClass::addProperty(
			propertyName,
			ss.str(),
			setter != 0 ? new PropertySet< ClassType, SetterValueType >(setter) : 0,
			getter != 0 ? new PropertyGet< ClassType, GetterValueType, true >(getter) : 0
		);
	}

	template < typename ValueType >
	void addProperty(const char* const propertyName, ValueType (ClassType::*getter)())
	{
		StringOutputStream ss;
		CastAny< ValueType >::typeName(ss);

		RuntimeClass::addProperty(
			propertyName,
			ss.str(),
			0,
			getter != 0 ? new PropertyGet< ClassType, ValueType, false >(getter) : 0
		);
	}

	template < typename SetterValueType, typename GetterValueType >
	void addProperty(const char* const propertyName, void (ClassType::*setter)(SetterValueType value), GetterValueType (ClassType::*getter)())
	{
		StringOutputStream ss;
		CastAny< GetterValueType >::typeName(ss);

		RuntimeClass::addProperty(
			propertyName,
			ss.str(),
			setter != 0 ? new PropertySet< ClassType, SetterValueType >(setter) : 0,
			getter != 0 ? new PropertyGet< ClassType, GetterValueType, false >(getter) : 0
		);
	}

	template < typename ValueType >
	void addProperty(const char* const propertyName, ValueType (*getter)(ClassType* self))
	{
		StringOutputStream ss;
		CastAny< ValueType >::typeName(ss);

		RuntimeClass::addProperty(
			propertyName,
			ss.str(),
			0,
			getter != 0 ? new FnPropertyGet< ClassType, ValueType >(getter) : 0
		);
	}

	template < typename SetterValueType, typename GetterValueType >
	void addProperty(const char* const propertyName, void (*setter)(ClassType* self, SetterValueType value), GetterValueType (*getter)(ClassType* self))
	{
		StringOutputStream ss;
		CastAny< GetterValueType >::typeName(ss);

		RuntimeClass::addProperty(
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
		int32_t opindex = -1;

		switch (operation)
		{
		case '+':
			opindex = OptAdd;
			break;

		case '-':
			opindex = OptSubtract;
			break;

		case '*':
			opindex = OptMultiply;
			break;

		case '/':
			opindex = OptDivide;
			break;

		default:
			T_FATAL_ERROR;
			break;
		}

		Ref< IRuntimeDispatch >& op = m_operators[opindex];
		if (op)
		{
			Ref< OperatorDispatch > opd = new OperatorDispatch();
			opd->add(op);
			opd->add(new Operator< ClassType, ReturnType, Argument1Type >(method));
			op = opd;
		}
		else
			op = new Operator< ClassType, ReturnType, Argument1Type >(method);
	}

	virtual const TypeInfo& getExportType() const override final
	{
		return type_of< ClassType >();
	}
};

//@}

}
