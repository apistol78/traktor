/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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

/*! Abstract entity factory.
 * \ingroup World
 */
class T_DLLCLASS AbstractEntityFactory : public IEntityFactory
{
	T_RTTI_CLASS;

public:
	virtual bool initialize(const ObjectStore& objectStore) override;

	virtual const TypeInfoSet getEntityTypes() const override;

	virtual const TypeInfoSet getEntityEventTypes() const override;

	virtual const TypeInfoSet getEntityComponentTypes() const override;

	virtual const TypeInfoSet getWorldComponentTypes() const override;

	virtual Ref< Entity > createEntity(const IEntityBuilder* builder, const EntityData& entityData) const override;

	virtual Ref< IEntityEvent > createEntityEvent(const IEntityBuilder* builder, const IEntityEventData& entityEventData) const override;

	virtual Ref< IEntityComponent > createEntityComponent(const IEntityBuilder* builder, const IEntityComponentData& entityComponentData) const override;

	virtual Ref< IWorldComponent > createWorldComponent(const IEntityBuilder* builder, const IWorldComponentData& worldComponentData) const override;
};

}
