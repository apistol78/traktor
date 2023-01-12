/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IPipelineCommon.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Heightfield/HeightfieldFormat.h"
#include "Heightfield/Editor/ConvertHeightfield.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Model/Model.h"
#include "Model/ModelCache.h"
#include "Physics/HeightfieldShapeDesc.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/StaticBodyDesc.h"
#include "Physics/Editor/MeshAsset.h"
#include "Physics/Editor/PhysicsEntityReplicator.h"
#include "Physics/World/RigidBodyComponentData.h"
#include "World/EntityData.h"

namespace traktor::physics
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.PhysicsEntityReplicator", 0, PhysicsEntityReplicator, scene::IEntityReplicator)

bool PhysicsEntityReplicator::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.AssetPath");
	m_modelCachePath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.ModelCache.Path");
	return true;
}

TypeInfoSet PhysicsEntityReplicator::getSupportedTypes() const
{
    return makeTypeInfoSet< RigidBodyComponentData >();
}

Ref< model::Model > PhysicsEntityReplicator::createVisualModel(
	editor::IPipelineCommon* pipelineCommon,
	const world::EntityData* entityData,
	const world::IEntityComponentData* componentData
) const
{
	return nullptr;
}

Ref< model::Model > PhysicsEntityReplicator::createCollisionModel(
	editor::IPipelineCommon* pipelineCommon,
	const world::EntityData* entityData,
	const world::IEntityComponentData* componentData
) const
{
	const RigidBodyComponentData* rigidBodyComponentData = mandatory_non_null_type_cast< const RigidBodyComponentData* >(componentData);

	auto bodyDesc = dynamic_type_cast< const StaticBodyDesc* >(rigidBodyComponentData->getBodyDesc());
	if (!bodyDesc)
		return nullptr;

	auto meshShape = dynamic_type_cast< const MeshShapeDesc* >(rigidBodyComponentData->getBodyDesc()->getShape());
	if (meshShape)
	{
		// Get referenced mesh asset.
		Ref< const physics::MeshAsset > meshAsset = pipelineCommon->getObjectReadOnly< physics::MeshAsset >(meshShape->getMesh());
		if (!meshAsset)
			return nullptr;

		// Read source model.
		Path filePath = FileSystem::getInstance().getAbsolutePath(Path(m_assetPath) + meshAsset->getFileName());
		Ref< model::Model > shapeModel = model::ModelCache(m_modelCachePath).get(filePath, meshAsset->getImportFilter());
		if (!shapeModel)
			return nullptr;

		return shapeModel;
	}

	auto heightfieldShape = dynamic_type_cast< const HeightfieldShapeDesc* >(rigidBodyComponentData->getBodyDesc()->getShape());
	if (heightfieldShape)
	{
		Ref< db::Instance > heightfieldAssetInstance = pipelineCommon->getSourceDatabase()->getInstance(heightfieldShape->getHeightfield());
		if (!heightfieldAssetInstance)
			return nullptr;

		Ref< const hf::HeightfieldAsset > heightfieldAsset = heightfieldAssetInstance->getObject< const hf::HeightfieldAsset >();
		if (!heightfieldAsset)
			return nullptr;

		Ref< IStream > sourceData = heightfieldAssetInstance->readData(L"Data");
		if (!sourceData)
			return nullptr;

		Ref< hf::Heightfield > heightfield = hf::HeightfieldFormat().read(
			sourceData,
			heightfieldAsset->getWorldExtent()
		);
		if (!heightfield)
			return nullptr;

		safeClose(sourceData);

		return hf::ConvertHeightfield().convert(heightfield, 4, heightfieldAsset->getVistaDistance());
	}

	return nullptr;
}

void PhysicsEntityReplicator::transform(
	world::EntityData* entityData,
	world::IEntityComponentData* componentData,
	world::GroupComponentData* outputGroup
) const
{
}

}
