/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Transform;

}

namespace traktor::world
{

class EntityData;

struct EntityState;

/*! Component data interface.
 * \ingroup World
 */
class T_DLLCLASS IEntityComponentData : public ISerializable
{
	T_RTTI_CLASS;

public:
	/*! Get this component's ordinal number.
	 *
	 * The ordinal number determine when in the sequence
	 * the component should be updated.
	 * The higher the number the later it will get
	 * updated.
	 *
	 * \return Ordinal number.
	 */
	virtual int32_t getOrdinal() const = 0;

	/*! Set the transform of the component data.
	 *
	 * \param owner Entity data owner of this component.
	 * \param transform New transform of entity.
	 */
	virtual void setTransform(const EntityData* owner, const Transform& transform) = 0;
};

}
