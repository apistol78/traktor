/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Shape/Editor/EntityFactory.h"
#include "Shape/Editor/Prefab/PrefabComponentData.h"
#include "Shape/Editor/Solid/PrimitiveEntity.h"
#include "Shape/Editor/Solid/PrimitiveEntityData.h"
#include "Shape/Editor/Solid/SolidEntity.h"
#include "Shape/Editor/Solid/SolidEntityData.h"
#include "Shape/Editor/Spline/ControlPointComponent.h"
#include "Shape/Editor/Spline/ControlPointComponentData.h"
#include "Shape/Editor/Spline/SplineComponent.h"
#include "Shape/Editor/Spline/SplineComponentData.h"
#include "Shape/Editor/Spline/SplineLayerComponent.h"
#include "Shape/Editor/Spline/SplineLayerComponentData.h"
#include "World/IEntityBuilder.h"

namespace traktor::shape
{
	namespace
	{
		
const resource::Id< render::Shader > c_defaultShader(Guid(L"{F01DE7F1-64CE-4613-9A17-899B44D5414E}"));

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.EntityFactory", EntityFactory, world::AbstractEntityFactory)

EntityFactory::EntityFactory(
	db::Database* database,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	physics::PhysicsManager* physicsManager,
	const std::wstring& assetPath,
	const std::wstring& modelCachePath
)
:	m_database(database)
,	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
,	m_physicsManager(physicsManager)
,	m_assetPath(assetPath)
,	m_modelCachePath(modelCachePath)
{
}

const TypeInfoSet EntityFactory::getEntityTypes() const
{
	return makeTypeInfoSet<
		PrimitiveEntityData,
		SolidEntityData
	>();
}

const TypeInfoSet EntityFactory::getEntityComponentTypes() const
{
	return makeTypeInfoSet<
		PrefabComponentData,
		ControlPointComponentData,
		SplineComponentData,
		SplineLayerComponentData
	>();
}

Ref< world::Entity > EntityFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	if (auto primitiveEntityData = dynamic_type_cast< const PrimitiveEntityData* >(&entityData))
		return primitiveEntityData->createEntity();
	else if (auto solidEntityData = dynamic_type_cast< const SolidEntityData* >(&entityData))
		return solidEntityData->createEntity(builder, m_resourceManager, m_renderSystem);
	else
		return nullptr;
}

Ref< world::IEntityComponent > EntityFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	if (auto controlPointData = dynamic_type_cast< const ControlPointComponentData* >(&entityComponentData))
		return controlPointData->createComponent();
	else if (auto splineLayerData = dynamic_type_cast< const SplineLayerComponentData* >(&entityComponentData))
	{
		return splineLayerData->createComponent(
			m_database,
			m_modelCachePath,
			m_assetPath
		);
	}
	else if (auto splineComponentData = dynamic_type_cast< const SplineComponentData* >(&entityComponentData))
	{
		resource::Proxy< render::Shader > shader;
		if (!m_resourceManager->bind(c_defaultShader, shader))
			return nullptr;

		return new SplineComponent(
			m_resourceManager,
			m_renderSystem,
			m_physicsManager,
			shader,
			splineComponentData
		);
	}
	else
	{
		// We're ignoring PrefabComponentData since it will not do anything in editor.
		return nullptr;
	}
}

}
