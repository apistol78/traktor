/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Database/Database.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/StaticBodyDesc.h"
#include "Physics/Editor/MeshAsset.h"
#include "Physics/World/RigidBodyComponentData.h"
#include "Render/Shader.h"
#include "Shape/Editor/Solid/PrimitiveComponentData.h"
#include "Shape/Editor/Solid/SolidComponentData.h"
#include "Shape/Editor/Solid/SolidComponentPipeline.h"
#include "Shape/Editor/Solid/SolidComponentReplicator.h"
#include "World/EntityData.h"

namespace traktor::shape
{
	namespace
	{

const resource::Id< render::Shader > c_defaultShader(Guid(L"{F01DE7F1-64CE-4613-9A17-899B44D5414E}"));

const Guid c_renderMeshIdSeed(L"{4082F1E0-F7BD-46DA-96D6-834B8A5E95B1}");
const Guid c_collisionShapeIdSeed(L"{3E0AF082-5A9D-4C50-86EB-F86EC5D99035}");

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.SolidComponentPipeline", 1, SolidComponentPipeline, world::EntityPipeline)

SolidComponentPipeline::SolidComponentPipeline()
:	m_targetEditor(false)
{
}

bool SolidComponentPipeline::create(const editor::IPipelineSettings* settings, db::Database* database)
{
	if (!world::EntityPipeline::create(settings, database))
		return false;

	m_targetEditor = settings->getPropertyIncludeHash< bool >(L"Pipeline.TargetEditor");
	return true;
}

TypeInfoSet SolidComponentPipeline::getAssetTypes() const
{
	return makeTypeInfoSet<
		PrimitiveComponentData,
		SolidComponentData
	>();
}

bool SolidComponentPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (m_targetEditor)
		pipelineDepends->addDependency(c_defaultShader, editor::PdfBuild | editor::PdfResource);

	if (auto solidComponentData = dynamic_type_cast< const SolidComponentData* >(sourceAsset))
	{
		for (auto id : solidComponentData->getCollisionGroup())
			pipelineDepends->addDependency(id, editor::PdfBuild | editor::PdfResource);
		for (auto id : solidComponentData->getCollisionMask())
			pipelineDepends->addDependency(id, editor::PdfBuild | editor::PdfResource);	
	}
	else if (auto primitiveComponentData = dynamic_type_cast< const PrimitiveComponentData* >(sourceAsset))
	{
		const auto& materials = primitiveComponentData->getMaterials();
		for (const auto& pair : materials)
			pipelineDepends->addDependency(pair.second, editor::PdfUse);
	}
	else
		return false;

	return world::EntityPipeline::buildDependencies(pipelineDepends, sourceInstance, sourceAsset, outputPath, outputGuid);
}

Ref< ISerializable > SolidComponentPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	if (auto solidComponentData = dynamic_type_cast< const SolidComponentData* >(sourceAsset))
	{
		const world::EntityData* entityData = mandatory_non_null_type_cast< const world::EntityData* >(buildParams);

		Ref< model::Model > outputModel = SolidComponentReplicator().createModel(pipelineBuilder, entityData, solidComponentData, world::IEntityReplicator::Usage::Visual);
		if (!outputModel)
			return nullptr;

		const Guid& entityId = entityData->getId();
		if (entityId.isNull())
			return nullptr;

		Guid outputRenderMeshGuid = entityId.permutation(c_renderMeshIdSeed);
		Guid outputCollisionShapeGuid = entityId.permutation(c_collisionShapeIdSeed);

		std::wstring outputRenderMeshPath = L"Generated/" + outputRenderMeshGuid.format();
		std::wstring outputCollisionShapePath = L"Generated/" + outputCollisionShapeGuid.format();

		// Create our output entity which will only contain the merged meshes.
		Ref< world::EntityData > outputEntityData = new world::EntityData();
		outputEntityData->setId(entityData->getId());
		outputEntityData->setName(entityData->getName());
		outputEntityData->setTransform(entityData->getTransform());

		// Build output mesh from merged model.
		Ref< mesh::MeshAsset > visualMeshAsset = new mesh::MeshAsset();
		visualMeshAsset->setMeshType(mesh::MeshAsset::MtStatic);
		pipelineBuilder->buildAdHocOutput(
			visualMeshAsset,
			outputRenderMeshPath,
			outputRenderMeshGuid,
			outputModel
		);

		// Replace mesh component referencing our merged mesh.
		outputEntityData->setComponent(new mesh::MeshComponentData(
			resource::Id< mesh::IMesh >(outputRenderMeshGuid)
		));

		// Build output mesh from merged model.
		Ref< physics::MeshAsset > physicsMeshAsset = new physics::MeshAsset();
		physicsMeshAsset->setMargin(0.0f);
		physicsMeshAsset->setCalculateConvexHull(false);
		pipelineBuilder->buildAdHocOutput(
			physicsMeshAsset,
			outputCollisionShapePath,
			outputCollisionShapeGuid,
			outputModel
		);

		// Replace mesh component referencing our merged physics mesh.
		Ref< physics::MeshShapeDesc > outputShapeDesc = new physics::MeshShapeDesc();
		outputShapeDesc->setMesh(resource::Id< physics::Mesh >(outputCollisionShapeGuid));
		outputShapeDesc->setCollisionGroup(solidComponentData->getCollisionGroup());
		outputShapeDesc->setCollisionMask(solidComponentData->getCollisionMask());

		Ref< physics::StaticBodyDesc > outputBodyDesc = new physics::StaticBodyDesc();
		outputBodyDesc->setShape(outputShapeDesc);

		outputEntityData->setComponent(new physics::RigidBodyComponentData(
			outputBodyDesc
		));

		return outputEntityData;
	}
	else
		return world::EntityPipeline::buildProduct(
			pipelineBuilder,
			sourceInstance,
			sourceAsset,
			buildParams
		);
}

}
