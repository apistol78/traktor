#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/ModelCache.h"
#include "Model/ModelFormat.h"
#include "Model/Operations/MergeModel.h"
#include "Model/Operations/Transform.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/StaticBodyDesc.h"
#include "Physics/Editor/MeshAsset.h"
#include "Physics/World/RigidBodyComponentData.h"
#include "Render/Editor/Texture/TextureSet.h"
#include "Scene/Editor/Traverser.h"
#include "Shape/Editor/Prefab/PrefabComponentData.h"
#include "Shape/Editor/Prefab/PrefabEntityReplicator.h"
#include "World/EntityData.h"
#include "World/Editor/EditorAttributesComponentData.h"

namespace traktor
{
	namespace shape
	{
		namespace
		{

const Guid c_shapeMeshAssetSeed(L"{FEC54BB1-1F55-48F5-AB87-58FE1712C42D}");

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.PrefabEntityReplicator", 1, PrefabEntityReplicator, scene::IEntityReplicator)

bool PrefabEntityReplicator::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.AssetPath");
	m_modelCachePath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.ModelCache.Path");
	return true;
}

TypeInfoSet PrefabEntityReplicator::getSupportedTypes() const
{
	return makeTypeInfoSet< PrefabComponentData >();
}

bool PrefabEntityReplicator::addDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const world::EntityData* entityData,
	const world::IEntityComponentData* componentData
) const
{
	const PrefabComponentData* prefabComponentData = mandatory_non_null_type_cast< const PrefabComponentData* >(componentData);
	scene::Traverser::visit(prefabComponentData, [&](const world::EntityData* entityData) -> scene::Traverser::VisitorResult
	{
		if (auto meshComponentData = entityData->getComponent< mesh::MeshComponentData >())
			pipelineDepends->addDependency(meshComponentData->getMesh(), editor::PdfUse);
		if (auto rigidBodyComponentData = entityData->getComponent< physics::RigidBodyComponentData >())
		{
			auto meshShape = dynamic_type_cast< const physics::MeshShapeDesc* >(rigidBodyComponentData->getBodyDesc()->getShape());
			if (meshShape)
				pipelineDepends->addDependency(meshShape->getMesh(), editor::PdfUse);
		}
		return scene::Traverser::VrContinue;
	});
	return true;
}

Ref< model::Model > PrefabEntityReplicator::createModel(
	editor::IPipelineBuilder* pipelineBuilder,
	const world::EntityData* entityData,
	const world::IEntityComponentData* componentData
) const
{
	const PrefabComponentData* prefabComponentData = mandatory_non_null_type_cast< const PrefabComponentData* >(componentData);
	Transform worldInv = entityData->getTransform().inverse();

	// Collect all models from prefab component.
	RefArray< model::Model > models;
	scene::Traverser::visit(prefabComponentData, [&](const world::EntityData* inEntityData) -> scene::Traverser::VisitorResult
	{
		// Check editor attributes component if we should include entity.
		if (auto editorAttributes = inEntityData->getComponent< world::EditorAttributesComponentData >())
		{
			if (!editorAttributes->include || editorAttributes->dynamic)
				return scene::Traverser::VrSkip;
		}		

		if (auto meshComponentData = inEntityData->getComponent< mesh::MeshComponentData >())
		{
			Ref< const mesh::MeshAsset > meshAsset = pipelineBuilder->getObjectReadOnly< mesh::MeshAsset >(
				meshComponentData->getMesh()
			);
			if (!meshAsset)
			{
				log::error << L"Prefab failed; unable to read mesh asset \"" << Guid(meshComponentData->getMesh()).format() << L"\"." << Endl;
				return scene::Traverser::VrFailed;
			}

			const Path filePath = FileSystem::getInstance().getAbsolutePath(Path(m_assetPath) + meshAsset->getFileName());

			Ref< model::Model > model = model::ModelCache(m_modelCachePath).get(filePath, meshAsset->getImportFilter());
			if (!model)
			{
				log::error << L"Prefab failed; unable to read model \"" << filePath.getPathName() << L"\"." << Endl;
				return scene::Traverser::VrFailed;
			}

			model::Transform(
				translate(meshAsset->getOffset()) *
				scale(meshAsset->getScaleFactor(), meshAsset->getScaleFactor(), meshAsset->getScaleFactor())
			).apply(*model);
			model::Transform(
				(worldInv * inEntityData->getTransform()).toMatrix44()
			).apply(*model);

			model->clear(model::Model::CfColors | model::Model::CfJoints);
			models.push_back(model);
		}

		return scene::Traverser::VrContinue;
	});

	// Create merged model.
	Ref< model::Model > outputModel = new model::Model();
	for (auto mdl : models)
		model::MergeModel(*mdl, Transform::identity(), 0.001f).apply(*outputModel);

	return outputModel;
}

Ref< Object > PrefabEntityReplicator::modifyOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const world::EntityData* entityData,
	const world::IEntityComponentData* componentData,
	const model::Model* model,
	const Guid& outputGuid
) const
{
	const PrefabComponentData* prefabComponentData = mandatory_non_null_type_cast< const PrefabComponentData* >(componentData);
	Transform worldInv = entityData->getTransform().inverse();

	// Collect all material textures, and also collect all collision
	// geometry if any.
	RefArray< const world::EntityData > dynamicEntityDatas;
	std::map< std::wstring, Guid > materialTemplates;
	std::map< std::wstring, Guid > materialTextures;
	std::map< std::wstring, Guid > materialPhysics;
	RefArray< const model::Model > shapeModels;
	std::set< resource::Id< physics::CollisionSpecification > > collisionGroup;
	std::set< resource::Id< physics::CollisionSpecification > > collisionMask;
	float friction = 0.0f;
	float restitution = 0.0f;

	scene::Traverser::visit(prefabComponentData, [&](const world::EntityData* inoutEntityData) -> scene::Traverser::VisitorResult
	{
		// Check editor attributes component if we should include entity.
		if (auto editorAttributes = inoutEntityData->getComponent< world::EditorAttributesComponentData >())
		{
			if (!editorAttributes->include)
				return scene::Traverser::VrSkip;
			if (editorAttributes->dynamic)
			{
				dynamicEntityDatas.push_back(inoutEntityData);
				return scene::Traverser::VrSkip;
			}
		}	

		if (auto meshComponentData = inoutEntityData->getComponent< mesh::MeshComponentData >())
		{
			Ref< const mesh::MeshAsset > meshAsset = pipelineBuilder->getObjectReadOnly< mesh::MeshAsset >(
				meshComponentData->getMesh()
			);
			if (!meshAsset)
			{
				log::error << L"Prefab failed; unable to read mesh asset \"" << Guid(meshComponentData->getMesh()).format() << L"\"." << Endl;
				return scene::Traverser::VrFailed;
			}

			// Insert material templates.
			for (const auto& mt : meshAsset->getMaterialTemplates())
				materialTemplates[mt.first] = mt.second;

			// First use textures from texture set.
			const auto& textureSetId = meshAsset->getTextureSet();
			if (textureSetId.isNotNull())
			{
				Ref< const render::TextureSet > textureSet = pipelineBuilder->getObjectReadOnly< render::TextureSet >(textureSetId);
				if (!textureSet)
				{
					log::error << L"Prefab failed; unable to read texture set \"" << textureSetId.format() << L"\"." << Endl;
					return scene::Traverser::VrFailed;
				}
				for (auto mt : textureSet->get())
					materialTextures[mt.first] = mt.second;
			}

			// Then let explicit material textures override those from a texture set.
			for (auto mt : meshAsset->getMaterialTextures())
				materialTextures[mt.first] = mt.second;
		}

		if (auto rigidBodyComponentData = inoutEntityData->getComponent< physics::RigidBodyComponentData >())
		{
			do
			{
				auto bodyDesc = dynamic_type_cast< const physics::StaticBodyDesc* >(rigidBodyComponentData->getBodyDesc());
				if (!bodyDesc)
					break;

				auto meshShape = dynamic_type_cast< const physics::MeshShapeDesc* >(rigidBodyComponentData->getBodyDesc()->getShape());
				if (!meshShape)
					break;

				Ref< const physics::MeshAsset > meshAsset = pipelineBuilder->getObjectReadOnly< physics::MeshAsset >(
					meshShape->getMesh()
				);
				if (!meshAsset)
				{
					log::error << L"Prefab failed; unable to read collision mesh \"" << Guid(meshShape->getMesh()).format() << L"\"." << Endl;
					return scene::Traverser::VrFailed;
				}

				Path filePath = FileSystem::getInstance().getAbsolutePath(Path(m_assetPath) + meshAsset->getFileName());

				Ref< model::Model > shapeModel = model::ModelCache(m_modelCachePath).get(filePath, L"");
				if (!shapeModel)
				{
					log::error << L"Prefab failed; unable to read collision model \"" << filePath.getPathName() << L"\"." << Endl;
					return scene::Traverser::VrFailed;
				}

				model::Transform(
					(worldInv * inoutEntityData->getTransform()).toMatrix44()
				).apply(*shapeModel);

				shapeModels.push_back(shapeModel);

				collisionGroup.insert(meshShape->getCollisionGroup().begin(), meshShape->getCollisionGroup().end());
				collisionMask.insert(meshShape->getCollisionMask().begin(), meshShape->getCollisionMask().end());

				for (auto it : meshAsset->getMaterials())
					materialPhysics[it.first] = it.second;

				friction += bodyDesc->getFriction();
				restitution += bodyDesc->getRestitution();
			}
			while (false);
		}

		return scene::Traverser::VrContinue;
	});

	// Create output group component containing merged meshes and rest of dynamic entities.
	Ref< world::GroupComponentData > outputGroup = new world::GroupComponentData();

	// Create and build a new mesh asset referencing the modified model.
	Ref< mesh::MeshAsset > outputMeshAsset = new mesh::MeshAsset();
	outputMeshAsset->setMeshType(mesh::MeshAsset::MtStatic);
	outputMeshAsset->setMaterialTemplates(materialTemplates);
	outputMeshAsset->setMaterialTextures(materialTextures);
	pipelineBuilder->buildAdHocOutput(
		outputMeshAsset,
		outputGuid,
		model
	);

	Ref< world::EntityData > outputMeshEntity = new world::EntityData();
	outputMeshEntity->setId(Guid::create());
	outputMeshEntity->setName(entityData->getName());
	outputMeshEntity->setTransform(entityData->getTransform());
	outputMeshEntity->setComponent(new mesh::MeshComponentData(resource::Id< mesh::IMesh >(outputGuid)));
	outputGroup->addEntityData(outputMeshEntity);

	if (!shapeModels.empty())
	{
		Guid outputShapeGuid = outputGuid.permutation(c_shapeMeshAssetSeed);

		// Create merged shape model.
		Ref< model::Model > outputModel = new model::Model();
		for (auto shapeModel : shapeModels)
			model::MergeModel(*shapeModel, Transform::identity(), 0.001f).apply(*outputModel);       

		// Build collision shape mesh.
		Ref< physics::MeshAsset > outputShapeMeshAsset = new physics::MeshAsset();
		outputShapeMeshAsset->setCalculateConvexHull(false);
		outputShapeMeshAsset->setMaterials(materialPhysics);
		pipelineBuilder->buildAdHocOutput(
			outputShapeMeshAsset,
			outputShapeGuid,
			outputModel
		);

		// Use average material properties as default for physics shape.
		friction /= (float)shapeModels.size();
		restitution /= (float)shapeModels.size();

		Ref< physics::MeshShapeDesc > outputShapeDesc = new physics::MeshShapeDesc(resource::Id< physics::Mesh >(outputShapeGuid));
		outputShapeDesc->setCollisionGroup(collisionGroup);
		outputShapeDesc->setCollisionMask(collisionMask);

		Ref< physics::StaticBodyDesc > outputBodyDesc = new physics::StaticBodyDesc(outputShapeDesc);
		outputBodyDesc->setFriction(friction);
		outputBodyDesc->setRestitution(restitution);

		Ref< world::EntityData > outputShapeMeshEntity = new world::EntityData();
		outputShapeMeshEntity->setId(Guid::create());
		outputShapeMeshEntity->setComponent(new physics::RigidBodyComponentData(outputBodyDesc));
		outputGroup->addEntityData(outputShapeMeshEntity);
	}

	// Add dynamic entities last.
	// \note Entity hierarchy is not preserved.
	for (auto dynamicEntityData : dynamicEntityDatas)
		outputGroup->addEntityData(DeepClone(dynamicEntityData).create< world::EntityData >());

	return outputGroup;
}

	}
}