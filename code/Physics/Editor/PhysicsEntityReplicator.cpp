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
#include "Core/Settings/PropertyObject.h"
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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.PhysicsEntityReplicator", 0, PhysicsEntityReplicator, world::IEntityReplicator)

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

RefArray< const world::IEntityComponentData > PhysicsEntityReplicator::getDependentComponents(
	const world::EntityData* entityData,
	const world::IEntityComponentData* componentData
) const
{
	RefArray< const world::IEntityComponentData > dependentComponentData;
	dependentComponentData.push_back(componentData);
	return dependentComponentData;
}

Ref< model::Model > PhysicsEntityReplicator::createModel(
	editor::IPipelineCommon* pipelineCommon,
	const world::EntityData* entityData,
	const world::IEntityComponentData* componentData,
	Usage usage
) const
{
	if (usage != Usage::Collision)
		return nullptr;

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
		Ref< model::Model > shapeModel = model::ModelCache::getInstance().getMutable(m_modelCachePath, filePath, meshAsset->getImportFilter());
		if (!shapeModel)
			return nullptr;

		// Attach information about the collision shape into the model.
 		Ref< physics::MeshAsset > outputShapeMeshAsset = new physics::MeshAsset();
 		outputShapeMeshAsset->setCalculateConvexHull(false);
		outputShapeMeshAsset->setMargin(meshAsset->getMargin());
 		outputShapeMeshAsset->setMaterials(meshAsset->getMaterials());
		shapeModel->setProperty< PropertyObject >(type_name(outputShapeMeshAsset), outputShapeMeshAsset);

		Ref< physics::ShapeDesc > outputShapeDesc = new physics::ShapeDesc();
		outputShapeDesc->setCollisionGroup(meshShape->getCollisionGroup());
		outputShapeDesc->setCollisionMask(meshShape->getCollisionMask());
		outputShapeDesc->setMaterial(meshShape->getMaterial());
		shapeModel->setProperty< PropertyObject >(type_name(outputShapeDesc), outputShapeDesc);

		Ref< physics::StaticBodyDesc > outputBodyDesc = new physics::StaticBodyDesc();
		outputBodyDesc->setFriction(bodyDesc->getFriction());
		outputBodyDesc->setRestitution(bodyDesc->getRestitution());
		shapeModel->setProperty< PropertyObject >(type_name(outputBodyDesc), outputBodyDesc);

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

		return hf::ConvertHeightfield().convert(heightfield, 4);
	}

	return nullptr;
}

}
