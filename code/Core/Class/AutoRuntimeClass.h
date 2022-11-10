/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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

/*! Automatic generation of script class definition.
 * \ingroup Core
 *
 * This class simplifies code necessary to map native classes
 * into script classes.
 * Just call addMethod with a pointer to your method and
 * it will automatically generate a "invoke" stub in compile
 * time.
 */
template < typename ClassType >
class AutoRuntimeClass final : public RuntimeClass
{
public:
	T_NO_COPY_CLASS(AutoRuntimeClass);

	AutoRuntimeClass() = default;

	void setUnknownHandler(typename Unknown< ClassType >::unknown_fn_t unknown)
	{
		m_unknown = new Unknown< ClassType >(unknown);
	}

	/*! \name Constructors */
	/*! \{ */

	template <
		typename ... ArgumentTypes
	>
	void addConstructor()
	{
		auto dispatch = new AutoConstructor< ClassType, ArgumentTypes ... >();
		RuntimeClass::addConstructor(sizeof ... (ArgumentTypes), dispatch);
	}

	template <
		typename ... ArgumentTypes
	>
	void addConstructor(typename AutoConstructorFactory< ClassType, ArgumentTypes ... >::factory_t factory)
	{
		auto dispatch = new AutoConstructorFactory< ClassType, ArgumentTypes ... >(factory);
		RuntimeClass::addConstructor(sizeof ... (ArgumentTypes), dispatch);
	}

	/*! \} */

	/*! \name Methods */
	/*! \{ */

	template <
		typename ReturnType,
		typename ... ArgumentTypes
	>
	void addMethod(const char* const methodName, ReturnType (ClassType::*method)(ArgumentTypes ...))
	{
		auto dispatch = new AutoMethod< false, ClassType, ReturnType, ArgumentTypes ... >(method);
		RuntimeClass::addMethod(methodName, sizeof...(ArgumentTypes), dispatch);
	}

	template <
		typename ReturnType,
		typename ... ArgumentTypes
	>
	void addMethod(const char* const methodName, ReturnType (ClassType::*method)(ArgumentTypes ...) const)
	{
		auto dispatch = new AutoMethod< true, ClassType, ReturnType, ArgumentTypes ... >(method);
		RuntimeClass::addMethod(methodName, sizeof...(ArgumentTypes), dispatch);
	}

	template <
		typename ReturnType,
		typename ... ArgumentTypes
	>
	void addMethod(const char* const methodName, ReturnType (*method)(ClassType*, ArgumentTypes ...))
	{
		auto dispatch = new AutoMethodTrunk< ClassType, ReturnType, ArgumentTypes ... >(method);
		RuntimeClass::addMethod(methodName, sizeof...(ArgumentTypes), dispatch);
	}

	/*! \} */

	/*! \name Static methods */
	/*! \{ */

	template <
		typename ReturnType,
		typename ... ArgumentTypes
	>
	void addStaticMethod(const char* const methodName, ReturnType (*method)(ArgumentTypes ...))
	{
		auto dispatch = new AutoStaticMethod< ClassType, ReturnType, ArgumentTypes ... >(method);
		RuntimeClass::addStaticMethod(methodName, sizeof...(ArgumentTypes), dispatch);
	}

	/*! \} */

	template < typename ValueType >
	void addProperty(const char* const propertyName, ValueType (ClassType::*getter)() const)
	{
		RuntimeClass::addProperty(
			propertyName,
			CastAny< ValueType >::typeName(),
			nullptr,
			getter != nullptr ? new PropertyGet< ClassType, ValueType, true >(getter) : nullptr
		);
	}

	template < typename SetterValueType, typename GetterValueType >
	void addProperty(const char* const propertyName, void (ClassType::*setter)(SetterValueType value), GetterValueType (ClassType::*getter)() const)
	{
		RuntimeClass::addProperty(
			propertyName,
			CastAny< GetterValueType >::typeName(),
			setter != nullptr ? new PropertySet< ClassType, SetterValueType >(setter) : nullptr,
			getter != nullptr ? new PropertyGet< ClassType, GetterValueType, true >(getter) : nullptr
		);
	}

	template < typename ValueType >
	void addProperty(const char* const propertyName, ValueType (ClassType::*getter)())
	{
		RuntimeClass::addProperty(
			propertyName,
			CastAny< ValueType >::typeName(),
			nullptr,
			getter != nullptr ? new PropertyGet< ClassType, ValueType, false >(getter) : nullptr
		);
	}

	template < typename SetterValueType, typename GetterValueType >
	void addProperty(const char* const propertyName, void (ClassType::*setter)(SetterValueType value), GetterValueType (ClassType::*getter)())
	{
		RuntimeClass::addProperty(
			propertyName,
			CastAny< GetterValueType >::typeName(),
			setter != nullptr ? new PropertySet< ClassType, SetterValueType >(setter) : nullptr,
			getter != nullptr ? new PropertyGet< ClassType, GetterValueType, false >(getter) : nullptr
		);
	}

	template < typename ValueType >
	void addProperty(const char* const propertyName, ValueType (*getter)(ClassType* self))
	{
		RuntimeClass::addProperty(
			propertyName,
			CastAny< ValueType >::typeName(),
			nullptr,
			getter != nullptr ? new FnPropertyGet< ClassType, ValueType >(getter) : nullptr
		);
	}

	template < typename SetterValueType, typename GetterValueType >
	void addProperty(const char* const propertyName, void (*setter)(ClassType* self, SetterValueType value), GetterValueType (*getter)(ClassType* self))
	{
		RuntimeClass::addProperty(
			propertyName,
			CastAny< GetterValueType >::typeName(),
			setter != nullptr ? new FnPropertySet< ClassType, SetterValueType >(setter) : nullptr,
			getter != nullptr ? new FnPropertyGet< ClassType, GetterValueType >(getter) : nullptr
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
			opindex = (int32_t)Operator::Add;
			break;

		case '-':
			opindex = (int32_t)Operator::Subtract;
			break;

		case '*':
			opindex = (int32_t)Operator::Multiply;
			break;

		case '/':
			opindex = (int32_t)Operator::Divide;
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
			opd->add(new traktor::Operator< ClassType, ReturnType, Argument1Type >(method));
			op = opd;
		}
		else
			op = new traktor::Operator< ClassType, ReturnType, Argument1Type >(method);
	}

	virtual const TypeInfo& getExportType() const override final
	{
		return type_of< ClassType >();
	}
};

//@}

}
