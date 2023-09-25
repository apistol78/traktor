/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/PolymorphicDispatch.h"
#include "Core/Class/RuntimeClass.h"

namespace traktor
{

void RuntimeClass::addConstant(const std::string& name, const Any& value)
{
	ConstInfo ci;
	ci.name = name;
	ci.value = value;
	m_consts.push_back(ci);
}

const IRuntimeDispatch* RuntimeClass::getConstructorDispatch() const
{
	return m_constructor;
}

uint32_t RuntimeClass::getConstantCount() const
{
	return uint32_t(m_consts.size());
}

std::string RuntimeClass::getConstantName(uint32_t constId) const
{
	return m_consts[constId].name;
}

Any RuntimeClass::getConstantValue(uint32_t constId) const
{
	return m_consts[constId].value;
}

uint32_t RuntimeClass::getMethodCount() const
{
	return uint32_t(m_methods.size());
}

std::string RuntimeClass::getMethodName(uint32_t methodId) const
{
	return m_methods[methodId].name;
}

const IRuntimeDispatch* RuntimeClass::getMethodDispatch(uint32_t methodId) const
{
	return m_methods[methodId].dispatch;
}

uint32_t RuntimeClass::getStaticMethodCount() const
{
	return uint32_t(m_staticMethods.size());
}

std::string RuntimeClass::getStaticMethodName(uint32_t methodId) const
{
	return m_staticMethods[methodId].name;
}

const IRuntimeDispatch* RuntimeClass::getStaticMethodDispatch(uint32_t methodId) const
{
	return m_staticMethods[methodId].dispatch;
}

uint32_t RuntimeClass::getPropertiesCount() const
{
	return uint32_t(m_properties.size());
}

std::string RuntimeClass::getPropertyName(uint32_t propertyId) const
{
	return m_properties[propertyId].name;
}

const IRuntimeDispatch* RuntimeClass::getPropertyGetDispatch(uint32_t propertyId) const
{
	return m_properties[propertyId].getter;
}

const IRuntimeDispatch* RuntimeClass::getPropertySetDispatch(uint32_t propertyId) const
{
	return m_properties[propertyId].setter;
}

const IRuntimeDispatch* RuntimeClass::getOperatorDispatch(Operator op) const
{
	return m_operators[(int32_t)op];
}

void RuntimeClass::addConstructor(uint32_t argc, IRuntimeDispatch* constructor)
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

void RuntimeClass::addMethod(const char* const methodName, uint32_t argc, IRuntimeDispatch* method)
{
	for (auto& methodInfo : m_methods)
	{
		if (methodInfo.name == methodName)
		{
			if (!is_a< PolymorphicDispatch >(methodInfo.dispatch))
			{
				Ref< PolymorphicDispatch > pd = new PolymorphicDispatch();
				pd->set(methodInfo.argc, methodInfo.dispatch);
				methodInfo.dispatch = pd;
			}
			mandatory_non_null_type_cast< PolymorphicDispatch* >(methodInfo.dispatch)->set(argc, method);
			return;
		}
	}

	MethodInfo& m = m_methods.push_back();
	m.name = methodName;
	m.argc = argc;
	m.dispatch = method;
}

void RuntimeClass::addStaticMethod(const char* const methodName, uint32_t argc, IRuntimeDispatch* method)
{
	for (auto& methodInfo : m_staticMethods)
	{
		if (methodInfo.name == methodName)
		{
			if (!is_a< PolymorphicDispatch >(methodInfo.dispatch))
			{
				Ref< PolymorphicDispatch > pd = new PolymorphicDispatch();
				pd->set(methodInfo.argc, methodInfo.dispatch);
				methodInfo.dispatch = pd;
			}
			mandatory_non_null_type_cast< PolymorphicDispatch* >(methodInfo.dispatch)->set(argc, method);
			return;
		}
	}

	MethodInfo& m = m_staticMethods.push_back();
	m.name = methodName;
	m.argc = argc;
	m.dispatch = method;
}

void RuntimeClass::addProperty(const char* const propertyName, const std::wstring& signature, IRuntimeDispatch* setter, IRuntimeDispatch* getter)
{
	for (auto& propertyInfo : m_properties)
	{
		if (propertyInfo.name == propertyName)
		{
			propertyInfo.setter = setter;
			propertyInfo.getter = getter;
			return;
		}
	}

	PropertyInfo& p = m_properties.push_back();
	p.name = propertyName;
	p.setter = setter;
	p.getter = getter;
}

}
