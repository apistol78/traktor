/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
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
#include "Core/Class/OperatorDispatch.h"
#include "Core/Class/RuntimeClass.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/Align.h"
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

		default: [[unlikely]]
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

	/*! \name Embedded value types */
	/*! \{ */

	/*! Mark instances as an immutable value type which script land may *embed*.
	 *
	 * Instances are then copied into script side storage instead of being allocated and
	 * reference counted.
	 *
	 * Only for classes that satisfy all of:
	 *  - the destructor has nothing to do (it is never run: the instance is released with
	 *    the storage it sits in), so the class must own no memory, references or handles;
	 *  - copy construction is a plain copy;
	 *  - instances are treated as immutable, since each push makes a separate copy.
	 */
	void setEmbeddedValueType()
	{
		static_assert(!std::is_abstract_v< ClassType >,
			"An abstract class cannot be embedded; it has no instances to copy.");
		static_assert(std::is_copy_constructible_v< ClassType >,
			"An embedded value type must be copy constructible.");

		// Slack for alignment to ensure SIMD alignment has enough room.
		RuntimeClass::setEmbeddedValue(
			(uint32_t)(sizeof(ClassType) + alignof(ClassType) - 1),
			&AutoRuntimeClass::embedCopyImpl
		);
	}

	/*! \} */

private:
	static ITypedObject* embedCopyImpl(void* storage, const ITypedObject* source)
	{
		uint8_t* at = (uint8_t*)alignUp(storage, alignof(ClassType));
		return ::new (at) ClassType(*(const ClassType*)source);
	}
};

//@}

}
