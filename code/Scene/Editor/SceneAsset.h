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
class IWorldComponentData;
class WorldRenderSettings;

}

namespace traktor::scene
{

class T_DLLCLASS SceneAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	SceneAsset();

	void setWorldRenderSettings(world::WorldRenderSettings* worldRenderSettings);

	Ref< world::WorldRenderSettings > getWorldRenderSettings() const;

	void setWorldComponents(const RefArray< world::IWorldComponentData >& worldComponents);

	const RefArray< world::IWorldComponentData >& getWorldComponents() const;

	/*! Get component of type.
	 *
	 * \param componentType Type of component.
	 * \return Component instance matching type.
	 */
	world::IWorldComponentData* getWorldComponent(const TypeInfo& componentType) const;

	/*! Get component of type.
	 *
	 * \param ComponentDataType Type of component.
	 * \return Component instance matching type.
	 */
	template < typename ComponentDataType >
	ComponentDataType* getWorldComponent() const
	{
		return checked_type_cast< ComponentDataType* >(getWorldComponent(type_of< ComponentDataType >()));
	}

	void setLayers(const RefArray< world::EntityData >& layers);

	const RefArray< world::EntityData >& getLayers() const;

	void setOperationData(const RefArray< ISerializable >& operationData);

	const RefArray< ISerializable >& getOperationData() const;

	virtual void serialize(ISerializer& s) override final;

private:
	Ref< world::WorldRenderSettings > m_worldRenderSettings;
	RefArray< world::IWorldComponentData > m_worldComponents;
	RefArray< world::EntityData > m_layers;
	RefArray< ISerializable > m_operationData;
};

}
