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
	/*! Add object to store.
	 * Any object of the same type gets replaced.
	 * \param object Object to add to store.
	 */
	void set(Object* object);

	/*! Remove object from store.
	 * \param object Object to remove from store.
	 * \return True if object was removed from store.
	 */
	bool unset(Object* object);

	/*! Get object of a specified type.
	 * \param objectType Type of object.
	 * \return Object which is of specified type.
	 */
	Object* get(const TypeInfo& objectType) const;

	/*! Get object of a specified type.
	 * \param ObjectType Type of object.
	 * \return Object which is of specified type.
	 */
	template < typename ObjectType >
	ObjectType* get() const
	{
		return dynamic_type_cast< ObjectType* >(get(type_of< ObjectType >()));
	}

	/*! Remove object of a specified type.
	 * \param ObjectType Type of object.
	 * \return True if an object was removed.
	 */
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
