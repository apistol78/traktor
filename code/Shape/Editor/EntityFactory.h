/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "World/IEntityFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::db
{

class Database;

}

namespace traktor::physics
{

class PhysicsManager;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::render
{

class IRenderSystem;

}

namespace traktor::shape
{

/*!
 * \ingroup Spray
 */
class T_DLLCLASS EntityFactory : public world::IEntityFactory
{
	T_RTTI_CLASS;

public:
	explicit EntityFactory(
		db::Database* database,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		physics::PhysicsManager* physicsManager,
		const std::wstring& assetPath,
		const std::wstring& modelCachePath
	);

	virtual const TypeInfoSet getEntityTypes() const override final;

	virtual const TypeInfoSet getEntityEventTypes() const override final;

	virtual const TypeInfoSet getEntityComponentTypes() const override final;

	virtual Ref< world::Entity > createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const override final;

	virtual Ref< world::IEntityEvent > createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const override final;

	virtual Ref< world::IEntityComponent > createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const override final;

private:
	Ref< db::Database > m_database;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< physics::PhysicsManager > m_physicsManager;
	std::wstring m_assetPath;
	std::wstring m_modelCachePath;
};

}
