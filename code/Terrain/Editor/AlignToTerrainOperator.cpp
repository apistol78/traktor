/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Terrain/Editor/AlignToTerrainOperator.h"

#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Random.h"
#include "Core/Math/Transform.h"
#include "Core/Math/Vector4.h"
#include "Core/Misc/SafeDestroy.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldFormat.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Scene/Editor/SceneAsset.h"
#include "Scene/Editor/Traverser.h"
#include "Terrain/TerrainComponentData.h"
#include "Terrain/Editor/AlignToTerrainOperationData.h"
#include "Terrain/Editor/TerrainAsset.h"
#include "World/EntityData.h"
#include "World/Entity/GroupComponentData.h"

namespace traktor::terrain
{
namespace
{

Guid findTerrainInScene(scene::SceneAsset* sceneAsset)
{
	Guid terrainId;
	for (auto layer : sceneAsset->getLayers())
	{
		if (!layer)
			continue;

		scene::Traverser::visit(layer, [&](const world::EntityData* entityData) -> scene::Traverser::Result {
			if (const auto terrainComponentData = entityData->getComponent< TerrainComponentData >())
			{
				if (terrainId.isNull())
					terrainId = terrainComponentData->getTerrain();
			}
			return scene::Traverser::Result::Continue;
		});

		if (terrainId.isNotNull())
			break;
	}
	return terrainId;
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.AlignToTerrainOperator", 0, AlignToTerrainOperator, scene::ISceneOperator)

bool AlignToTerrainOperator::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void AlignToTerrainOperator::destroy()
{
}

TypeInfoSet AlignToTerrainOperator::getOperatorTypes() const
{
	return makeTypeInfoSet< AlignToTerrainOperationData >();
}

void AlignToTerrainOperator::addDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const ISerializable* operatorData
) const
{
}

bool AlignToTerrainOperator::isGeometricTransform() const
{
	return true;
}

bool AlignToTerrainOperator::transform(
	const scene::ISceneOperator::TransformContext& context,
	const ISerializable* operatorData,
	scene::SceneAsset* inoutSceneAsset
) const
{
	const AlignToTerrainOperationData* data = mandatory_non_null_type_cast< const AlignToTerrainOperationData* >(operatorData);

	const Guid terrainId = findTerrainInScene(inoutSceneAsset);
	if (terrainId.isNull())
	{
		log::warning << L"AlignToTerrain; no terrain found in scene, nothing aligned." << Endl;
		return true;
	}

	// #fixme
	// Should probably give access to Scene instance from scene editor to
	// allow querying terrain component without going through the database.
	//
	Ref< const TerrainAsset > terrainAsset = context.getObjectReadOnly< TerrainAsset >(terrainId);
	if (!terrainAsset)
		return true;

	Ref< db::Instance > heightfieldInstance = context.getSourceDatabase()->getInstance(terrainAsset->getHeightfield());
	if (!heightfieldInstance)
		return true;

	Ref< const hf::HeightfieldAsset > heightfieldAsset = heightfieldInstance->getObject< const hf::HeightfieldAsset >();
	if (!heightfieldAsset)
		return true;

	Ref< IStream > sourceData = heightfieldInstance->readData(L"Data");
	if (!sourceData)
		return true;

	Ref< hf::Heightfield > heightfield = hf::HeightfieldFormat().read(
		sourceData,
		heightfieldAsset->getWorldExtent()
	);
	safeClose(sourceData);
	if (!heightfield)
		return true;
	// #fixme

	const AlignedVector< std::wstring >& layerFilters = data->getLayers();
	Random rndm;

	uint32_t aligned = 0;
	for (auto layer : inoutSceneAsset->getLayers())
	{
		if (!layer)
			continue;

		if (std::find(layerFilters.begin(), layerFilters.end(), layer->getName()) == layerFilters.end())
			continue;

		auto group = layer->getComponent< world::GroupComponentData >();
		if (!group)
			continue;

		for (auto entityData : group->getEntityData())
		{
			if (!entityData)
				continue;

			if (entityData->getComponent< TerrainComponentData >() != nullptr)
				continue;

			const Transform current = entityData->getTransform();
			const Vector4 position = current.translation();
			const float worldX = position.x();
			const float worldZ = position.z();
			const float worldY = heightfield->getWorldHeight(worldX, worldZ) + data->getOffset();

			Quaternion rotation = current.rotation();

			if (data->getAlignOrientation())
			{
				float gridX, gridZ;
				heightfield->worldToGrid(worldX, worldZ, gridX, gridZ);
				const Vector4 normal = heightfield->normalAt(gridX, gridZ);
				rotation = slerp(
					Quaternion(Vector4(0.0f, 1.0f, 0.0f, 0.0f), normal),
					Quaternion::identity(),
					data->getUpness()
				);
			}

			if (data->getRandomHeadingAngle())
			{
				const float rnd = rndm.nextFloat() * TWO_PI;
				rotation = rotation * Quaternion::fromEulerAngles(rnd, 0.0f, 0.0f);
			}

			entityData->setTransform(Transform(
				Vector4(worldX, worldY, worldZ, 1.0f),
				rotation
			));
			++aligned;
		}
	}

	log::debug << L"AlignToTerrain; aligned " << aligned << L" entities to terrain." << Endl;
	return true;
}

bool AlignToTerrainOperator::build(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* operatorData,
	const db::Instance* sourceInstance,
	scene::SceneAsset* inoutSceneAsset,
	bool rebuild
) const
{
	// All alignment happens in transform(); nothing to build here.
	return true;
}

}
