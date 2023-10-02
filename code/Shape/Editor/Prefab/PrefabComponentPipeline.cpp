/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyObject.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/DataAccessCache.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Editor/Pipeline/PipelineProfiler.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/ShapeDesc.h"
#include "Physics/StaticBodyDesc.h"
#include "Physics/Editor/MeshAsset.h"
#include "Physics/World/RigidBodyComponentData.h"
#include "Shape/Editor/Prefab/PrefabComponentData.h"
#include "Shape/Editor/Prefab/PrefabComponentPipeline.h"
#include "Shape/Editor/Prefab/PrefabEntityReplicator.h"
#include "World/EntityData.h"
#include "World/Entity/GroupComponentData.h"
#include "World/Editor/ResolveExternal.h"

namespace traktor::shape
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.PrefabComponentPipeline", 2, PrefabComponentPipeline, world::EntityPipeline)

bool PrefabComponentPipeline::create(const editor::IPipelineSettings* settings)
{
	m_editor = settings->getPropertyIncludeHash< bool >(L"Pipeline.TargetEditor", false);

	m_replicator = new PrefabEntityReplicator();
	if (!m_replicator->create(settings))
		return false;

	return true;
}

TypeInfoSet PrefabComponentPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< PrefabComponentData >();
}

Ref< ISerializable > PrefabComponentPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	// Do not build prefab in editor since it's time consuming and not
	// strictly necessary for editing purposes.
	//if (m_editor)
	//{
	//	return world::EntityPipeline::buildProduct(
	//		pipelineBuilder,
	//		sourceInstance,
	//		sourceAsset,
	//		buildParams
	//	);
	//}

	const world::EntityData* entityData = mandatory_non_null_type_cast< const world::EntityData* >(buildParams);
	const PrefabComponentData* prefabComponent = mandatory_non_null_type_cast< const PrefabComponentData* >(sourceAsset);

	// Resolve all external entities, replicator doesn't handle external meshes.
	Ref< world::EntityData > resolvedEntityData = checked_type_cast< world::EntityData* >(world::resolveExternal(
		[&](const Guid& objectId) -> Ref< const ISerializable > {
			return pipelineBuilder->getObjectReadOnly(objectId);
		},
		entityData,
		Guid::null,
		nullptr
	));
	if (!resolvedEntityData)
		return nullptr;

	// Create models through replicator.
	const RefArray< const world::IEntityComponentData > dependentComponentData = m_replicator->getDependentComponents(resolvedEntityData, prefabComponent);
	if (dependentComponentData.empty())
		return nullptr;

	uint32_t componentDataHash = 0;
	for (auto cd : dependentComponentData)
	{
		T_FATAL_ASSERT(cd != prefabComponent);
		componentDataHash += pipelineBuilder->calculateInclusiveHash(cd);
	}

	Ref< model::Model > visualModel = pipelineBuilder->getDataAccessCache()->read< model::Model >(
		Key(0x00000100, 0x00000000, type_of(m_replicator).getVersion(), componentDataHash),
		[&]() -> Ref< model::Model > {
			pipelineBuilder->getProfiler()->begin(type_of(m_replicator));
			Ref< model::Model > model = m_replicator->createModel(pipelineBuilder, resolvedEntityData, prefabComponent, world::IEntityReplicator::Usage::Visual);
			pipelineBuilder->getProfiler()->end();
			return model;
		}
	);

	Ref< model::Model > collisionModel = pipelineBuilder->getDataAccessCache()->read< model::Model >(
		Key(0x00000200, 0x00000000, type_of(m_replicator).getVersion(), componentDataHash),
		[&]() -> Ref< model::Model > {
			pipelineBuilder->getProfiler()->begin(type_of(m_replicator));
			Ref< model::Model > model = m_replicator->createModel(pipelineBuilder, resolvedEntityData, prefabComponent, world::IEntityReplicator::Usage::Collision);
			pipelineBuilder->getProfiler()->end();
			return model;
		}
	);

	if (!visualModel && !collisionModel)
		return nullptr;

	Ref< world::EntityData > replacementEntityData = new world::EntityData();
	replacementEntityData->setId(entityData->getId());
	replacementEntityData->setName(entityData->getName());
	replacementEntityData->setTransform(entityData->getTransform());

	if (visualModel)
	{
		// Create an ad-hoc mesh asset.
		const Guid outputMeshId = entityData->getId();
		log::info << entityData->getName() << L" -> " << outputMeshId.format() << Endl;

		Ref< const mesh::MeshAsset > meshAsset = dynamic_type_cast< const mesh::MeshAsset* >(
			visualModel->getProperty< ISerializable >(type_name< mesh::MeshAsset >())
		);

		// Create and build a new mesh asset referencing the modified model.
		Ref< mesh::MeshAsset > outputMeshAsset = new mesh::MeshAsset();
		outputMeshAsset->setMeshType(mesh::MeshAsset::MtStatic);
		if (meshAsset)
		{
			outputMeshAsset->setMaterialTemplates(meshAsset->getMaterialTemplates());
			outputMeshAsset->setMaterialShaders(meshAsset->getMaterialShaders());
			outputMeshAsset->setMaterialTextures(meshAsset->getMaterialTextures());
		}

		// Create static mesh component.
		replacementEntityData->setComponent(new mesh::MeshComponentData(resource::Id< mesh::IMesh >(outputMeshId)));

		// Ensure visual mesh is build.
		pipelineBuilder->buildAdHocOutput(
			outputMeshAsset,
			outputMeshId,
			visualModel
		);
	}

	if (collisionModel)
	{
		const Guid outputShapeId = entityData->getId().permutation(1);

		Ref< const physics::MeshAsset > meshAsset = dynamic_type_cast< const physics::MeshAsset* >(
			collisionModel->getProperty< ISerializable >(type_name< physics::MeshAsset >())
		);

		Ref< const physics::ShapeDesc > shapeDesc = dynamic_type_cast< const physics::ShapeDesc* >(
			collisionModel->getProperty< ISerializable >(type_name< physics::ShapeDesc >())
		);

		Ref< const physics::StaticBodyDesc > bodyDesc = dynamic_type_cast< const physics::StaticBodyDesc* >(
			collisionModel->getProperty< ISerializable >(type_name< physics::StaticBodyDesc >())
		);

		// Build collision shape mesh.
		Ref< physics::MeshAsset > outputMeshAsset = new physics::MeshAsset();
		outputMeshAsset->setCalculateConvexHull(false);
		if (meshAsset)
			outputMeshAsset->setMaterials(meshAsset->getMaterials());

		Ref< physics::MeshShapeDesc > outputShapeDesc = new physics::MeshShapeDesc(resource::Id< physics::Mesh >(outputShapeId));
		if (shapeDesc)
		{
			outputShapeDesc->setCollisionGroup(shapeDesc->getCollisionGroup());
			outputShapeDesc->setCollisionMask(shapeDesc->getCollisionMask());
		}

		Ref< physics::StaticBodyDesc > outputBodyDesc = new physics::StaticBodyDesc(outputShapeDesc);
		if (bodyDesc)
		{
			outputBodyDesc->setFriction(bodyDesc->getFriction());
			outputBodyDesc->setRestitution(bodyDesc->getRestitution());
		}

		replacementEntityData->setComponent(new physics::RigidBodyComponentData(outputBodyDesc));

		// Ensure collision shape is built.
		pipelineBuilder->buildAdHocOutput(
			outputMeshAsset,
			outputShapeId,
			collisionModel
		);
	}

	return replacementEntityData;
}

}
