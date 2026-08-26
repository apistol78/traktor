/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Theater/RuntimeProperties.h"

#include "Core/Class/IRuntimeClass.h"
#include "Core/Class/IRuntimeClassFactory.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Misc/TString.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/Thread/Acquire.h"

#include <algorithm>

namespace traktor::theater
{
	namespace
	{

class CollectClassRegistrar : public IRuntimeClassRegistrar
{
public:
	explicit CollectClassRegistrar(RefArray< IRuntimeClass >& outRuntimeClasses)
	:	m_runtimeClasses(outRuntimeClasses)
	{
	}

	virtual void registerClass(IRuntimeClass* runtimeClass) override final
	{
		m_runtimeClasses.push_back(runtimeClass);
	}

private:
	RefArray< IRuntimeClass >& m_runtimeClasses;
};

	}

RuntimeProperties& RuntimeProperties::getInstance()
{
	static RuntimeProperties* s_instance = nullptr;
	if (!s_instance)
	{
		s_instance = new RuntimeProperties();
		SingletonManager::getInstance().add(s_instance);
	}
	return *s_instance;
}

void RuntimeProperties::getProperties(const TypeInfo& type, AlignedVector< Property >& outProperties)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	outProperties = resolve(type);
}

bool RuntimeProperties::findProperty(const TypeInfo& type, const std::wstring& name, Property& outProperty)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	const AlignedVector< Property >& properties = resolve(type);
	const auto it = std::find_if(properties.begin(), properties.end(), [&](const Property& property) {
		return property.name == name;
	});
	if (it == properties.end())
		return false;

	outProperty = *it;
	return true;
}

void RuntimeProperties::destroy()
{
	delete this;
}

const AlignedVector< RuntimeProperties::Property >& RuntimeProperties::resolve(const TypeInfo& type)
{
	// Class factories are found through RTTI, thus only loaded modules are collected.
	if (!m_collected)
	{
		m_collected = true;

		CollectClassRegistrar registrar(m_runtimeClasses);
		for (const auto runtimeClassFactoryType : type_of< IRuntimeClassFactory >().findAllOf(false))
		{
			Ref< IRuntimeClassFactory > runtimeClassFactory = dynamic_type_cast< IRuntimeClassFactory* >(runtimeClassFactoryType->createInstance());
			if (runtimeClassFactory)
				runtimeClassFactory->createClasses(&registrar);
		}

		for (auto runtimeClass : m_runtimeClasses)
			m_classes[&runtimeClass->getExportType()] = runtimeClass;
	}

	const auto it = m_properties.find(&type);
	if (it != m_properties.end())
		return it->second;

	AlignedVector< Property >& properties = m_properties[&type];

	// A class only export properties of its own type; derived types shadow their base.
	for (const TypeInfo* t = &type; t != nullptr; t = t->getSuper())
	{
		const auto itc = m_classes.find(t);
		if (itc == m_classes.end())
			continue;

		const IRuntimeClass* runtimeClass = itc->second;
		const uint32_t propertyCount = runtimeClass->getPropertiesCount();
		for (uint32_t i = 0; i < propertyCount; ++i)
		{
			// Only a property which can be both read and written can be animated.
			const IRuntimeDispatch* getter = runtimeClass->getPropertyGetDispatch(i);
			const IRuntimeDispatch* setter = runtimeClass->getPropertySetDispatch(i);
			if (getter == nullptr || setter == nullptr)
				continue;

			const std::wstring name = mbstows(runtimeClass->getPropertyName(i));
			const auto itp = std::find_if(properties.begin(), properties.end(), [&](const Property& property) {
				return property.name == name;
			});
			if (itp != properties.end())
				continue;

			properties.push_back({ name, getter, setter });
		}
	}

	return properties;
}

}
