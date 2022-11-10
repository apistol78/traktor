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
#include "Core/Reflection/RfmCompound.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;

/*! Object data reflection.
 * \ingroup Core
 *
 * Using serialization mechanism to create a
 * reflection structure which can be queried
 * or altered in a type independent manor.
 *
 * \note
 * The reflection is shallow, thus only reflect
 * the first level of objects. Deeper level objects
 * are exposed as a RfmObject member descriptor.
 */
class T_DLLCLASS Reflection : public RfmCompound
{
	T_RTTI_CLASS;

public:
	/*! Create a reflection model from object.
	 *
	 * \param object Source object.
	 * \return Reflection model.
	 */
	static Ref< Reflection > create(const ISerializable* object);

	/*! Apply changes on model back onto object.
	 *
	 * \param object Target object.
	 * \return True if changes successfully applied.
	 */
	bool apply(ISerializable* object) const;

	/*! Use model to create a clone of the source object.
	 *
	 * \param memory Instantiate into memory.
	 * \return Clone of source object.
	 */
	Ref< ISerializable > clone(void* memory = nullptr) const;

private:
	const TypeInfo& m_objectType;

	Reflection(const TypeInfo& objectType);
};

}

