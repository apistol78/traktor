/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class EntityData;

/*! Group component persistent data.
 * \ingroup World
 */
class T_DLLCLASS GroupComponentData : public IEntityComponentData
{
	T_RTTI_CLASS;

public:
	GroupComponentData() = default;

	explicit GroupComponentData(const RefArray< EntityData >& entityData);

	void addEntityData(EntityData* entityData);

	void addEntityData(EntityData* insertAfter, EntityData* entityData);

	void removeEntityData(EntityData* entityData);

	void removeAllEntityData();

	void setEntityData(const RefArray< EntityData >& entityData);

	RefArray< EntityData >& getEntityData();

	const RefArray< EntityData >& getEntityData() const;

	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const EntityData* owner, const Transform& transform) override;

	virtual void serialize(ISerializer& s) override;

private:
    RefArray< EntityData > m_entityData;
};

}
