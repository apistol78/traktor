/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
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

namespace traktor::render
{
	
class IRenderSystem;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::world
{

/*! World entity factory.
 * \ingroup World
 */
class T_DLLCLASS WorldEntityFactory : public IEntityFactory
{
	T_RTTI_CLASS;

public:
	explicit WorldEntityFactory(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		bool editor
	);

	virtual const TypeInfoSet getEntityTypes() const override final;

	virtual const TypeInfoSet getEntityEventTypes() const override final;

	virtual const TypeInfoSet getEntityComponentTypes() const override final;

	virtual Ref< Entity > createEntity(const IEntityBuilder* builder, const EntityData& entityData) const override final;

	virtual Ref< IEntityEvent > createEntityEvent(const IEntityBuilder* builder, const IEntityEventData& entityEventData) const override final;

	virtual Ref< IEntityComponent > createEntityComponent(const world::IEntityBuilder* builder, const IEntityComponentData& entityComponentData) const override final;

private:
	mutable Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	bool m_editor;
};

}
