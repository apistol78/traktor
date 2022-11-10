/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Model/ModelCache.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Shape/Editor/EntityFactory.h"
#include "Shape/Editor/Solid/PrimitiveEntity.h"
#include "Shape/Editor/Solid/PrimitiveEntityData.h"
#include "Shape/Editor/Solid/SolidEntity.h"
#include "Shape/Editor/Solid/SolidEntityData.h"
#include "Shape/Editor/Spline/ControlPointComponent.h"
#include "Shape/Editor/Spline/ControlPointComponentData.h"
#include "Shape/Editor/Spline/SplineEntity.h"
#include "Shape/Editor/Spline/SplineEntityData.h"
#include "Shape/Editor/Spline/SplineLayerComponent.h"
#include "Shape/Editor/Spline/SplineLayerComponentData.h"
#include "World/IEntityBuilder.h"

namespace traktor
{
	namespace shape
	{
		namespace
		{
		
const resource::Id< render::Shader > c_defaultShader(Guid(L"{F01DE7F1-64CE-4613-9A17-899B44D5414E}"));

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.EntityFactory", EntityFactory, world::IEntityFactory)

EntityFactory::EntityFactory(
	db::Database* database,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const std::wstring& assetPath,
	const std::wstring& modelCachePath
)
:	m_database(database)
,	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
,	m_assetPath(assetPath)
,	m_modelCachePath(modelCachePath)
{
}

const TypeInfoSet EntityFactory::getEntityTypes() const
{
	return makeTypeInfoSet<
		PrimitiveEntityData,
		SolidEntityData,
		SplineEntityData
	>();
}

const TypeInfoSet EntityFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet EntityFactory::getEntityComponentTypes() const
{
	return makeTypeInfoSet<
		ControlPointComponentData,
		SplineLayerComponentData
	>();
}

Ref< world::Entity > EntityFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	if (auto primitiveEntityData = dynamic_type_cast< const PrimitiveEntityData* >(&entityData))
		return primitiveEntityData->createEntity();
	else if (auto solidEntityData = dynamic_type_cast< const SolidEntityData* >(&entityData))
		return solidEntityData->createEntity(builder, m_resourceManager, m_renderSystem);
	else if (auto splineEntityData = dynamic_type_cast< const SplineEntityData* >(&entityData))
	{
		resource::Proxy< render::Shader > shader;
		if (!m_resourceManager->bind(c_defaultShader, shader))
			return nullptr;

		Ref< model::ModelCache > modelCache = new model::ModelCache(m_modelCachePath);
		Ref< SplineEntity > entity = new SplineEntity(
			splineEntityData,
			m_database,
			m_renderSystem,
			modelCache,
			m_assetPath,
			shader
		);

		entity->setTransform(entityData.getTransform());

		// Instantiate all components.
		RefArray< world::IEntityComponent > components;
		for (auto componentData : entityData.getComponents())
		{
			Ref< world::IEntityComponent > component = builder->create(componentData);
			if (!component)
				continue;
			entity->setComponent(component);
		}

		return entity;
	}
	else
		return nullptr;
}

Ref< world::IEntityEvent > EntityFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return nullptr;
}

Ref< world::IEntityComponent > EntityFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	if (auto controlPointData = dynamic_type_cast< const ControlPointComponentData* >(&entityComponentData))
		return controlPointData->createComponent();
	else if (auto splineLayerData = dynamic_type_cast< const SplineLayerComponentData* >(&entityComponentData))
		return splineLayerData->createComponent(m_database);
	else
		return nullptr;
}

	}
}
