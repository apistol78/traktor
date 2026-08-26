/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Singleton/ISingleton.h"
#include "Core/Thread/Semaphore.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_THEATER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IRuntimeClass;
class IRuntimeDispatch;
class TypeInfo;

}

namespace traktor::theater
{

/*! Properties exported through runtime classes.
 * \ingroup Theater
 *
 * Classes are collected, lazily, from all registered class factories as there
 * is no global registry of runtime classes.
 */
class T_DLLCLASS RuntimeProperties : public ISingleton
{
public:
	/*! Property which can be both read and written. */
	struct Property
	{
		std::wstring name;
		const IRuntimeDispatch* getter = nullptr;
		const IRuntimeDispatch* setter = nullptr;
	};

	static RuntimeProperties& getInstance();

	/*! Get all properties, which have both a getter and a setter, of a type and its base types. */
	void getProperties(const TypeInfo& type, AlignedVector< Property >& outProperties);

	/*! Find named property of a type; false if the type has none. */
	bool findProperty(const TypeInfo& type, const std::wstring& name, Property& outProperty);

protected:
	virtual void destroy() override final;

private:
	Semaphore m_lock;
	RefArray< IRuntimeClass > m_runtimeClasses;
	SmallMap< const TypeInfo*, const IRuntimeClass* > m_classes;
	SmallMap< const TypeInfo*, AlignedVector< Property > > m_properties;
	bool m_collected = false;

	/*! Resolve properties of a type and its base types; called with lock held. */
	const AlignedVector< Property >& resolve(const TypeInfo& type);
};

}
