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
#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"
#include "World/WorldTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class EntityData;
class WorldRenderSettings;

}

namespace traktor::scene
{

class ISceneControllerData;

class T_DLLCLASS SceneAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	SceneAsset();

	void setWorldRenderSettings(world::WorldRenderSettings* worldRenderSettings);

	Ref< world::WorldRenderSettings > getWorldRenderSettings() const;

	void setLayers(const RefArray< world::EntityData >& layers);

	const RefArray< world::EntityData >& getLayers() const;

	void setControllerData(ISceneControllerData* controllerData);

	Ref< ISceneControllerData > getControllerData() const;

	void setOperationData(const RefArray< ISerializable >& operationData);

	const RefArray< ISerializable >& getOperationData() const;

	virtual void serialize(ISerializer& s) override final;

private:
	Ref< world::WorldRenderSettings > m_worldRenderSettings;
	RefArray< world::EntityData > m_layers;
	Ref< ISceneControllerData > m_controllerData;
	RefArray< ISerializable > m_operationData;
};

}
