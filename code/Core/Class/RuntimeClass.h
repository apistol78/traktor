/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Class/IRuntimeClass.h"
#include "Core/Containers/AlignedVector.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IRuntimeDispatch;

class T_DLLCLASS RuntimeClass : public IRuntimeClass
{
public:
	T_NO_COPY_CLASS(RuntimeClass);

	RuntimeClass() = default;

	/*! \name Constants */
	/*! \{ */

	void addConstant(const std::string& name, const Any& value);

	/*! \} */

	virtual const IRuntimeDispatch* getConstructorDispatch() const override final;

	virtual uint32_t getConstantCount() const override final;

	virtual std::string getConstantName(uint32_t constId) const override final;

	virtual Any getConstantValue(uint32_t constId) const override final;

	virtual uint32_t getMethodCount() const override final;

	virtual std::string getMethodName(uint32_t methodId) const override final;

	virtual const IRuntimeDispatch* getMethodDispatch(uint32_t methodId) const override final;

	virtual uint32_t getStaticMethodCount() const override final;

	virtual std::string getStaticMethodName(uint32_t methodId) const override final;

	virtual const IRuntimeDispatch* getStaticMethodDispatch(uint32_t methodId) const override final;

	virtual uint32_t getPropertiesCount() const override final;

	virtual std::string getPropertyName(uint32_t propertyId) const override final;

	const IRuntimeDispatch* getPropertyGetDispatch(uint32_t propertyId) const override final;

	const IRuntimeDispatch* getPropertySetDispatch(uint32_t propertyId) const override final;

	virtual const IRuntimeDispatch* getOperatorDispatch(Operator op) const override final;

protected:
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

	uint32_t m_constructorArgc = 0;
	Ref< IRuntimeDispatch > m_constructor;
	AlignedVector< ConstInfo > m_consts;
	AlignedVector< MethodInfo > m_methods;
	AlignedVector< MethodInfo > m_staticMethods;
	AlignedVector< PropertyInfo > m_properties;
	Ref< IRuntimeDispatch > m_operators[(int32_t)Operator::Count];

	void addConstructor(uint32_t argc, IRuntimeDispatch* constructor);

	void addMethod(const char* const methodName, uint32_t argc, IRuntimeDispatch* method);

	void addStaticMethod(const char* const methodName, uint32_t argc, IRuntimeDispatch* method);

	void addProperty(const char* const propertyName, const std::wstring& signature, IRuntimeDispatch* setter, IRuntimeDispatch* getter);
};

}
