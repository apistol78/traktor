/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Terrain/Editor/TerrainCollisionOperator.h"

#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineSettings.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Physics/BodyDesc.h"
#include "Physics/HeightfieldShapeDesc.h"
#include "Physics/World/RigidBodyComponentData.h"
#include "Scene/Editor/SceneAsset.h"
#include "Terrain/Editor/TerrainAsset.h"
#include "Terrain/Editor/TerrainCollisionOperationData.h"
#include "Terrain/Editor/TerrainPipeline.h"
#include "Terrain/TerrainComponentData.h"
#include "World/Entity/EntityStreamingComponentData.h"
#include "World/Entity/GroupComponentData.h"
#include "World/EntityData.h"
#include "World/IEntityEventData.h"

#include <algorithm>

namespace traktor::terrain
{
namespace
{

const Guid c_guidTileEntitySeed(L"{5E0C2B7A-93D1-4F6E-A8B4-1C7D20E95F36}");

//! Heightfield shape of entity's rigid body, null if entity has no heightfield collision.
const physics::HeightfieldShapeDesc* getHeightfieldShape(const world::EntityData* entityData)
{
	auto rigidBodyComponentData = entityData->getComponent< physics::RigidBodyComponentData >();
	if (!rigidBodyComponentData || !rigidBodyComponentData->getBodyDesc())
		return nullptr;
	return dynamic_type_cast< const physics::HeightfieldShapeDesc* >(rigidBodyComponentData->getBodyDesc()->getShape());
}

//! Id of collision entity of terrain tile.
Guid tileEntityId(const Guid& terrainEntityId, int32_t tileX, int32_t tileZ)
{
	uint8_t d[16];
	for (int i = 0; i < 16; ++i)
		d[i] = c_guidTileEntitySeed[i] ^ terrainEntityId[i];
	d[0] ^= (uint8_t)tileX;
	d[1] ^= (uint8_t)(tileX >> 8);
	d[2] ^= (uint8_t)tileZ;
	d[3] ^= (uint8_t)(tileZ >> 8);
	return Guid(d);
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.TerrainCollisionOperator", 0, TerrainCollisionOperator, scene::ISceneOperator)

bool TerrainCollisionOperator::create(const editor::IPipelineSettings* settings)
{
	m_targetEditor = settings->getPropertyIncludeHash< bool >(L"Pipeline.TargetEditor", false);
	return true;
}

void TerrainCollisionOperator::destroy()
{
}

TypeInfoSet TerrainCollisionOperator::getOperatorTypes() const
{
	return makeTypeInfoSet< TerrainCollisionOperationData >();
}

void TerrainCollisionOperator::addDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const ISerializable* operatorData) const
{
}

bool TerrainCollisionOperator::transform(
	const scene::ISceneOperator::TransformContext& context,
	const ISerializable* operatorData,
	scene::SceneAsset* inoutSceneAsset) const
{
	return true;
}

bool TerrainCollisionOperator::build(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* operatorData,
	const db::Instance* sourceInstance,
	scene::SceneAsset* inoutSceneAsset,
	bool rebuild) const
{
	const auto data = mandatory_non_null_type_cast< const TerrainCollisionOperationData* >(operatorData);

	// Editor doesn't stream entities, terrain keeps collision of entire heightfield.
	if (m_targetEditor)
		return true;

	RefArray< world::EntityData > terrainEntities;
	RefArray< world::GroupComponentData > parents;
	for (auto layer : inoutSceneAsset->getLayers())
	{
		if (!layer || !layer->getState().visible)
			continue;
		if (auto groupComponentData = layer->getComponent< world::GroupComponentData >())
			findTerrainEntities(groupComponentData, terrainEntities, parents);
	}
	if (terrainEntities.empty())
		return true;

	// Tiles are added as cells of entity streaming, cells are grown so tiles are loaded within load distance.
	world::EntityStreamingComponentData* entityStreaming = inoutSceneAsset->getWorldComponent< world::EntityStreamingComponentData >();
	AlignedVector< world::EntityStreamingComponentData::Cell > cells;
	float margin = 0.0f;
	if (entityStreaming)
	{
		cells = entityStreaming->getCells();
		margin = std::max(data->getLoadDistance() - entityStreaming->getLoadDistance(), 0.0f);
	}

	int32_t tileEntityCount = 0;
	for (size_t i = 0; i < terrainEntities.size(); ++i)
	{
		world::EntityData* terrainEntity = terrainEntities[i];

		const auto terrainComponentData = terrainEntity->getComponent< TerrainComponentData >();
		const auto rigidBodyComponentData = terrainEntity->getComponent< physics::RigidBodyComponentData >();
		const auto heightfieldShape = getHeightfieldShape(terrainEntity);

		Ref< const TerrainAsset > terrainAsset = pipelineBuilder->getObjectReadOnly< TerrainAsset >(terrainComponentData->getTerrain());
		if (!terrainAsset)
		{
			log::error << L"Terrain collision operator failed; unable to read terrain asset of \"" << terrainEntity->getName() << L"\"." << Endl;
			return false;
		}

		// Only collision with terrain's own heightfield is replaced.
		if (heightfieldShape->getHeightfield() != terrainAsset->getHeightfield())
			continue;

		Ref< const hf::HeightfieldAsset > heightfieldAsset = pipelineBuilder->getObjectReadOnly< hf::HeightfieldAsset >(terrainAsset->getHeightfield());
		if (!heightfieldAsset)
		{
			log::error << L"Terrain collision operator failed; unable to read heightfield asset of \"" << terrainEntity->getName() << L"\"." << Endl;
			return false;
		}

		const int32_t tileCount = std::max(heightfieldAsset->getTileCount(), 1);
		const Vector4& worldExtent = heightfieldAsset->getWorldExtent();
		const Vector4 tileExtent(worldExtent.x() / tileCount, worldExtent.y(), worldExtent.z() / tileCount, 0.0f);
		const Vector4 grow(margin, margin, margin, 0.0f);

		Ref< world::IEntityEventData > eventCollide;
		if (rigidBodyComponentData->getEventCollide())
			eventCollide = DeepClone(rigidBodyComponentData->getEventCollide()).create< world::IEntityEventData >();

		for (int32_t tz = 0; tz < tileCount; ++tz)
		{
			for (int32_t tx = 0; tx < tileCount; ++tx)
			{
				Ref< physics::HeightfieldShapeDesc > shape = DeepClone(heightfieldShape).create< physics::HeightfieldShapeDesc >();
				shape->setHeightfield(resource::Id< hf::Heightfield >(TerrainPipeline::getTileHeightfieldId(terrainComponentData->getTerrain(), tx, tz)));

				Ref< physics::BodyDesc > bodyDesc = DeepClone(rigidBodyComponentData->getBodyDesc()).create< physics::BodyDesc >();
				bodyDesc->setShape(shape);

				Ref< world::EntityData > tileEntity = new world::EntityData();
				tileEntity->setId(tileEntityId(terrainEntity->getId(), tx, tz));
				tileEntity->setName(terrainEntity->getName() + L" collision " + toString(tx) + L"_" + toString(tz));
				tileEntity->setTransform(terrainEntity->getTransform());
				tileEntity->setComponent(new physics::RigidBodyComponentData(bodyDesc, eventCollide));

				if (entityStreaming)
				{
					const Vector4 mn = (-worldExtent * 0.5_simd + Vector4(float(tx), 0.0f, float(tz), 0.0f) * tileExtent).xyz1();
					const Aabb3 boundingBox = Aabb3(mn, mn + tileExtent).transform(terrainEntity->getTransform());

					auto& cell = cells.push_back();
					cell.boundingBox = Aabb3(boundingBox.mn - grow, boundingBox.mx + grow);
					cell.entityData.push_back(tileEntity);
				}
				else
					parents[i]->addEntityData(tileEntity);

				++tileEntityCount;
			}
		}

		terrainEntity->removeComponent(rigidBodyComponentData);
	}

	if (entityStreaming)
		entityStreaming->setCells(cells);

	log::info << L"Terrain collision; " << tileEntityCount << L" tile entities." << Endl;
	return true;
}

void TerrainCollisionOperator::findTerrainEntities(world::GroupComponentData* groupComponentData, RefArray< world::EntityData >& outEntities, RefArray< world::GroupComponentData >& outParents) const
{
	for (auto entityData : groupComponentData->getEntityData())
	{
		if (!entityData || !entityData->getState().visible)
			continue;

		if (entityData->getComponent< TerrainComponentData >() != nullptr && getHeightfieldShape(entityData) != nullptr)
		{
			outEntities.push_back(entityData);
			outParents.push_back(groupComponentData);
		}

		if (auto childGroupComponentData = entityData->getComponent< world::GroupComponentData >())
			findTerrainEntities(childGroupComponentData, outEntities, outParents);
	}
}

}
