/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#pragma once

#include "Core/Class/AutoConstructor.h"
#include "Core/Class/AutoMethod.h"
#include "Core/Class/AutoMethodTrunk.h"
#include "Core/Class/AutoOperator.h"
#include "Core/Class/AutoProperty.h"
#include "Core/Class/AutoStaticMethod.h"
#include "Core/Class/AutoUnknown.h"
#include "Core/Class/IRuntimeClass.h"
#include "Core/Class/OperatorDispatch.h"
#include "Core/Class/PolymorphicDispatch.h"
#include "Core/Containers/AlignedVector.h"
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
class AutoRuntimeClass : public IRuntimeClass
{
public:
	T_NO_COPY_CLASS(AutoRuntimeClass);

	AutoRuntimeClass()
	{
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

	void setUnknownHandler(typename Unknown< ClassType >::unknown_fn_t unknown)
	{
		m_unknown = new Unknown< ClassType >(unknown);
	}

	virtual const TypeInfo& getExportType() const T_OVERRIDE T_FINAL
	{
		return type_of< ClassType >();
	}

	virtual const IRuntimeDispatch* getConstructorDispatch() const T_OVERRIDE T_FINAL
	{
		return m_constructor;
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

	virtual const IRuntimeDispatch* getMethodDispatch(uint32_t methodId) const T_OVERRIDE T_FINAL
	{
		return m_methods[methodId].dispatch;
	}

	virtual uint32_t getStaticMethodCount() const T_OVERRIDE T_FINAL
	{
		return uint32_t(m_staticMethods.size());
	}

	virtual std::string getStaticMethodName(uint32_t methodId) const T_OVERRIDE T_FINAL
	{
		return m_staticMethods[methodId].name;
	}

	virtual const IRuntimeDispatch* getStaticMethodDispatch(uint32_t methodId) const T_OVERRIDE T_FINAL
	{
		return m_staticMethods[methodId].dispatch;
	}

	virtual uint32_t getPropertiesCount() const T_OVERRIDE T_FINAL
	{
		return uint32_t(m_properties.size());
	}

	virtual std::string getPropertyName(uint32_t propertyId) const T_OVERRIDE T_FINAL
	{
		return m_properties[propertyId].name;
	}

	const IRuntimeDispatch* getPropertyGetDispatch(uint32_t propertyId) const
	{
		return m_properties[propertyId].getter;
	}

	const IRuntimeDispatch* getPropertySetDispatch(uint32_t propertyId) const
	{
		return m_properties[propertyId].setter;
	}

	virtual const IRuntimeDispatch* getOperatorDispatch(OperatorType op) const T_OVERRIDE T_FINAL
	{
		return m_operators[op];
	}

	virtual const IRuntimeDispatch* getUnknownDispatch() const T_OVERRIDE T_FINAL
	{
		return m_unknown;
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
		uint32_t argc;
		Ref< IRuntimeDispatch > dispatch;
	};

	struct PropertyInfo
	{
		std::string name;
		Ref< IRuntimeDispatch > setter;
		Ref< IRuntimeDispatch > getter;
	};

	uint32_t m_constructorArgc;
	Ref< IRuntimeDispatch > m_constructor;
	AlignedVector< ConstInfo > m_consts;
	AlignedVector< MethodInfo > m_methods;
	AlignedVector< MethodInfo > m_staticMethods;
	AlignedVector< PropertyInfo > m_properties;
	Ref< IRuntimeDispatch > m_operators[OptCount];
	Ref< IRuntimeDispatch > m_unknown;

	void addConstructor(uint32_t argc, IRuntimeDispatch* constructor)
	{
		if (m_constructor)
		{
			if (!is_a< PolymorphicDispatch >(m_constructor))
			{
				Ref< PolymorphicDispatch > pd = new PolymorphicDispatch();
				pd->set(m_constructorArgc, m_constructor);
				m_constructor = pd;
			}
			mandatory_non_null_type_cast< PolymorphicDispatch* >(m_constructor)->set(argc, constructor);
			return;
		}

		m_constructor = constructor;
		m_constructorArgc = argc;
	}

	void addMethod(const std::string& methodName, uint32_t argc, IRuntimeDispatch* method)
	{
		for (typename AlignedVector< MethodInfo >::iterator i = m_methods.begin(); i != m_methods.end(); ++i)
		{
			if (i->name == methodName)
			{
				if (!is_a< PolymorphicDispatch >(i->dispatch))
				{
					Ref< PolymorphicDispatch > pd = new PolymorphicDispatch();
					pd->set(i->argc, i->dispatch);
					i->dispatch = pd;
				}
				mandatory_non_null_type_cast< PolymorphicDispatch* >(i->dispatch)->set(argc, method);
				return;
			}
		}

		MethodInfo& m = m_methods.push_back();
		m.name = methodName;
		m.argc = argc;
		m.dispatch = method;
	}

	void addStaticMethod(const std::string& methodName, uint32_t argc, IRuntimeDispatch* method)
	{
		for (typename AlignedVector< MethodInfo >::iterator i = m_staticMethods.begin(); i != m_staticMethods.end(); ++i)
		{
			if (i->name == methodName)
			{
				if (!is_a< PolymorphicDispatch >(i->dispatch))
				{
					Ref< PolymorphicDispatch > pd = new PolymorphicDispatch();
					pd->set(i->argc, i->dispatch);
					i->dispatch = pd;
				}
				mandatory_non_null_type_cast< PolymorphicDispatch* >(i->dispatch)->set(argc, method);
				return;
			}
		}

		MethodInfo& m = m_staticMethods.push_back();
		m.name = methodName;
		m.argc = argc;
		m.dispatch = method;
	}

	void addProperty(const std::string& propertyName, const std::wstring& signature, IRuntimeDispatch* setter, IRuntimeDispatch* getter)
	{
		for (typename AlignedVector< PropertyInfo >::iterator i = m_properties.begin(); i != m_properties.end(); ++i)
		{
			if (i->name == propertyName)
			{
				i->setter = setter;
				i->getter = getter;
				return;
			}
		}

		PropertyInfo& p = m_properties.push_back();
		p.name = propertyName;
		p.setter = setter;
		p.getter = getter;
	}
};

//@}

}
