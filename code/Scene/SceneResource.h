/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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

namespace traktor::world
{

class IEntityBuilder;
class EntityData;
class WorldRenderSettings;

}

namespace traktor::scene
{

class ISceneControllerData;
class Scene;

/*! Scene resource.
 * \ingroup Scene
 */
class T_DLLCLASS SceneResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	SceneResource();

	Ref< Scene > createScene(world::IEntityBuilder* entityBuilder) const;

	void setWorldRenderSettings(world::WorldRenderSettings* worldRenderSettings);

	Ref< world::WorldRenderSettings > getWorldRenderSettings() const;

	void setEntityData(world::EntityData* entityData);

	Ref< world::EntityData > getEntityData() const;

	void setControllerData(ISceneControllerData* controllerData);

	Ref< ISceneControllerData > getControllerData() const;

	virtual void serialize(ISerializer& s) override final;

private:
	Ref< world::WorldRenderSettings > m_worldRenderSettings;
	Ref< world::EntityData > m_entityData;
	Ref< ISceneControllerData > m_controllerData;
};

}
