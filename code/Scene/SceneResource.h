/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"
#include "World/WorldRenderSettings.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::render
{

class IRenderSystem;

}

namespace traktor::world
{

class IEntityFactory;
class IWorldComponentData;
class EntityData;
class WorldRenderSettings;

}

namespace traktor::scene
{

class Scene;

/*! Scene resource.
 * \ingroup Scene
 */
class T_DLLCLASS SceneResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	SceneResource();

	Ref< Scene > createScene(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const world::IEntityFactory* entityFactory) const;

	void setWorldRenderSettings(world::WorldRenderSettings* worldRenderSettings);

	Ref< world::WorldRenderSettings > getWorldRenderSettings() const;

	void setWorldComponents(const RefArray< world::IWorldComponentData >& worldComponents);

	const RefArray< world::IWorldComponentData >& getWorldComponents() const;
	
	void setEntityData(world::EntityData* entityData);

	Ref< world::EntityData > getEntityData() const;

	virtual void serialize(ISerializer& s) override final;

private:
	Ref< world::WorldRenderSettings > m_worldRenderSettings;
	RefArray< world::IWorldComponentData > m_worldComponents;
	Ref< world::EntityData > m_entityData;
};

}
