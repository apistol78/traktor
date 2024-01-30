/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "World/IEntityBuilder.h"

namespace traktor::world
{

class IEntityFactory;
class World;

}

namespace traktor::scene
{

class EntityAdapter;
class SceneEditorContext;

class EntityAdapterBuilder : public world::IEntityBuilder
{
	T_RTTI_CLASS;

public:
	explicit EntityAdapterBuilder(
		SceneEditorContext* context,
		const world::IEntityFactory* entityFactory,
		world::World* world,
		EntityAdapter* currentEntityAdapter
	);

	virtual ~EntityAdapterBuilder();

	virtual Ref< world::Entity > create(const world::EntityData* entityData) const override final;

	virtual Ref< world::IEntityEvent > create(const world::IEntityEventData* entityEventData) const override final;

	virtual Ref< world::IEntityComponent > create(const world::IEntityComponentData* entityComponentData) const override final;

	EntityAdapter* getRootAdapter() const { return m_rootAdapter; }

	uint32_t getCacheHit() const { return m_cacheHit; }

	uint32_t getCacheMiss() const { return m_cacheMiss; }

private:
	struct Cache
	{
		Ref< EntityAdapter > adapter;
		uint32_t leafEntityHash;
		Ref< world::Entity > leafEntity;
	};

	SceneEditorContext* m_context;
	//Ref< world::IEntityBuilder > m_entityBuilder;

	Ref< const world::IEntityFactory > m_entityFactory;
	Ref< world::World > m_world;

	mutable SmallMap< Guid, Cache > m_cache;
	mutable Ref< EntityAdapter > m_currentAdapter;
	mutable Ref< EntityAdapter > m_rootAdapter;
	mutable Ref< const world::EntityData > m_currentEntityData;
	mutable uint32_t m_cacheHit;
	mutable uint32_t m_cacheMiss;
};

}
