/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Object store container, maps type to instance.
 * \ingroup Core
 */
class T_DLLCLASS ObjectStore : public Object
{
	T_RTTI_CLASS;

public:
	void set(Object* object);

	bool unset(Object* object);

	Object* get(const TypeInfo& objectType) const;

	template < typename ObjectType >
	ObjectType* get() const
	{
		return dynamic_type_cast< ObjectType* >(get(type_of< ObjectType >()));
	}

	template < typename ObjectType >
	bool unset()
	{
		Object* object = get< ObjectType >();
		return object ? unset(object) : false;
	}

private:
	RefArray< Object > m_objects;
};

}
