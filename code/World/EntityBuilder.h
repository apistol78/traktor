/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "World/IEntityBuilder.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class IEntityFactory;
class World;

/*! Entity builder.
 * \ingroup World
 */
class T_DLLCLASS EntityBuilder : public IEntityBuilder
{
	T_RTTI_CLASS;

public:
	explicit EntityBuilder(const IEntityFactory* entityFactory, World* world);

	virtual Ref< Entity > create(const EntityData* entityData) const override final;

	virtual Ref< IEntityEvent > create(const IEntityEventData* entityEventData) const override final;

	virtual Ref< IEntityComponent > create(const IEntityComponentData* entityComponentData) const override final;

	virtual Ref< IWorldComponent > create(const IWorldComponentData* worldComponentData) const override final;

private:
	Ref< const IEntityFactory > m_entityFactory;
	Ref< world::World > m_world;
};

}
