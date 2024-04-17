/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "World/IEntityFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

/*! Entity factory facade for multiple factories.
 * \ingroup World
 */
class T_DLLCLASS EntityFactory : public IEntityFactory
{
	T_RTTI_CLASS;

public:
	void addFactory(const IEntityFactory* entityFactory);

	void removeFactory(const IEntityFactory* entityFactory);

	virtual const TypeInfoSet getEntityTypes() const override final;

	virtual const TypeInfoSet getEntityEventTypes() const override final;

	virtual const TypeInfoSet getEntityComponentTypes() const override final;

	virtual const TypeInfoSet getWorldComponentTypes() const override final;

	virtual Ref< Entity > createEntity(const IEntityBuilder* builder, const EntityData& entityData) const override final;

	virtual Ref< IEntityEvent > createEntityEvent(const IEntityBuilder* builder, const IEntityEventData& entityEventData) const override final;

	virtual Ref< IEntityComponent > createEntityComponent(const IEntityBuilder* builder, const IEntityComponentData& entityComponentData) const override final;

	virtual Ref< IWorldComponent > createWorldComponent(const IEntityBuilder* builder, const IWorldComponentData& worldComponentData) const override final;

private:
	RefArray< const IEntityFactory > m_factories;
	mutable SmallMap< const TypeInfo*, const IEntityFactory* > m_resolvedFactoryCache;

	const IEntityFactory* getFactory(const EntityData* entityData) const;

	const IEntityFactory* getFactory(const IEntityEventData* entityEventData) const;

	const IEntityFactory* getFactory(const IEntityComponentData* entityComponentData) const;

	const IEntityFactory* getFactory(const IWorldComponentData* worldComponentData) const;
};

}
