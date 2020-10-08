#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/ModelCache.h"
#include "Model/Operations/CleanDegenerate.h"
#include "Model/Operations/CleanDuplicates.h"
#include "Model/Operations/MergeCoplanarAdjacents.h"
#include "Model/Operations/MergeModel.h"
#include "Model/Operations/Quantize.h"
#include "Model/Operations/Transform.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/StaticBodyDesc.h"
#include "Physics/Editor/MeshAsset.h"
#include "Physics/World/RigidBodyComponentData.h"
#include "Shape/Editor/Prefab/PrefabEntityData.h"
#include "Shape/Editor/Prefab/PrefabEntityPipeline.h"
#include "Shape/Editor/Prefab/PrefabMerge.h"
#include "World/EntityData.h"
#include "World/Entity/ExternalEntityData.h"

namespace traktor
{
	namespace shape
	{
		namespace
		{

Ref< ISerializable > resolveAllExternal(editor::IPipelineCommon* pipeline, const ISerializable* object)
{
	Ref< Reflection > reflection = Reflection::create(object);

	RefArray< ReflectionMember > objectMembers;
	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);

	while (!objectMembers.empty())
	{
		Ref< RfmObject > objectMember = checked_type_cast< RfmObject*, false >(objectMembers.front());
		objectMembers.pop_front();

		if (const world::ExternalEntityData* externalEntityDataRef = dynamic_type_cast< const world::ExternalEntityData* >(objectMember->get()))
		{
			Ref< const ISerializable > externalEntityData = pipeline->getObjectReadOnly(externalEntityDataRef->getEntityData());
			if (!externalEntityData)
			{
				log::error << L"PrefabEntityPipeline failed; Unable to read external entity." << Endl;
				return nullptr;
			}

			Ref< world::EntityData > resolvedEntityData = dynamic_type_cast< world::EntityData* >(resolveAllExternal(pipeline, externalEntityData));
			if (!resolvedEntityData)
			{
				log::error << L"PrefabEntityPipeline failed; Unable to resolve external entity." << Endl;
				return nullptr;
			}

			resolvedEntityData->setName(externalEntityDataRef->getName());
			resolvedEntityData->setTransform(externalEntityDataRef->getTransform());

			objectMember->set(resolvedEntityData);
		}
		else if (objectMember->get())
		{
			objectMember->set(resolveAllExternal(pipeline, objectMember->get()));
		}
	}

	return reflection->clone();
}

void collectComponentEntities(const ISerializable* object, RefArray< world::EntityData >& outComponentEntities)
{
	Ref< Reflection > reflection = Reflection::create(object);

	RefArray< ReflectionMember > objectMembers;
	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);

	while (!objectMembers.empty())
	{
		Ref< RfmObject > objectMember = checked_type_cast< RfmObject*, false >(objectMembers.front());
		objectMembers.pop_front();

		if (world::EntityData* entityData = dynamic_type_cast< world::EntityData* >(objectMember->get()))
		{
			outComponentEntities.push_back(entityData);
		}
		else if (objectMember->get())
		{
			collectComponentEntities(objectMember->get(), outComponentEntities);
		}
	}
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.PrefabEntityPipeline", 1, PrefabEntityPipeline, world::EntityPipeline)

PrefabEntityPipeline::PrefabEntityPipeline()
:	m_visualMeshSnap(0.01f)
,	m_collisionMeshSnap(0.01f)
,	m_mergeCoplanar(true)
{
}

bool PrefabEntityPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	m_modelCachePath = settings->getProperty< std::wstring >(L"Pipeline.ModelCachePath", L"");
	m_visualMeshSnap = settings->getProperty< float >(L"PrefabPipeline.VisualMeshSnap", 0.01f);
	m_collisionMeshSnap = settings->getProperty< float >(L"PrefabPipeline.CollisionMeshSnap", 0.01f);
	m_mergeCoplanar = settings->getProperty< bool >(L"PrefabPipeline.MergeCoplanar", true);
	return true;
}

TypeInfoSet PrefabEntityPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< PrefabEntityData >();
}

bool PrefabEntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	Ref< PrefabEntityData > prefabEntityData = checked_type_cast< PrefabEntityData* >(resolveAllExternal(pipelineDepends, sourceAsset));
	if (!prefabEntityData)
	{
		log::error << L"Prefab entity pipeline failed; Unable to resolve all external entities." << Endl;
		return false;
	}

	// Get all component entities which contain visual and/or physics meshes.
	RefArray< world::EntityData > entityDatas;
	collectComponentEntities(prefabEntityData, entityDatas);

	for (auto entityData : entityDatas)
	{
		const mesh::MeshComponentData* meshComponent = entityData->getComponent< mesh::MeshComponentData >();
		if (meshComponent)
			pipelineDepends->addDependency(meshComponent->getMesh(), editor::PdfUse);

		const physics::RigidBodyComponentData* rigidBodyComponent = entityData->getComponent< physics::RigidBodyComponentData >();
		if (rigidBodyComponent)
		{
			const physics::BodyDesc* bodyDesc = rigidBodyComponent->getBodyDesc();
			if (bodyDesc)
			{
				const physics::MeshShapeDesc* meshShapeDesc = dynamic_type_cast< const physics::MeshShapeDesc* >(bodyDesc->getShape());
				if (meshShapeDesc)
					pipelineDepends->addDependency(meshShapeDesc->getMesh(), editor::PdfUse);
			}
		}
	}

	return true;
}

Ref< ISerializable > PrefabEntityPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	Ref< PrefabEntityData > prefabEntityData = checked_type_cast< PrefabEntityData* >(resolveAllExternal(pipelineBuilder, sourceAsset));
	if (!prefabEntityData)
	{
		log::error << L"Prefab entity pipeline failed; Unable to resolve all external entities." << Endl;
		return nullptr;
	}

	model::ModelCache modelCache(
		m_modelCachePath,
		[&](const Path& p) {
			return pipelineBuilder->getFile(p);
		},
		[&](const Path& p) {
			return pipelineBuilder->openFile(p);
		}
	);

	Guid outputRenderMeshGuid = pipelineBuilder->synthesizeOutputGuid(1);
	Guid outputCollisionShapeGuid = pipelineBuilder->synthesizeOutputGuid(1);

	std::wstring outputRenderMeshPath = L"Generated/" + outputRenderMeshGuid.format();
	std::wstring outputCollisionShapePath = L"Generated/" + outputCollisionShapeGuid.format();

	// Get all component entities which contain visual and/or physics meshes.
	RefArray< world::EntityData > entityDatas;
	collectComponentEntities(prefabEntityData, entityDatas);

	Ref< PrefabMerge > merge = new PrefabMerge();

	std::set< resource::Id< physics::CollisionSpecification > > shapeCollisionGroup;
	std::set< resource::Id< physics::CollisionSpecification > > shapeCollisionMask;

	Transform Tprefab = prefabEntityData->getTransform();
	Transform TprefabInv = Tprefab.inverse();

	for (auto entityData : entityDatas)
	{
		const mesh::MeshComponentData* meshComponent = entityData->getComponent< mesh::MeshComponentData >();
		if (meshComponent)
		{
			Ref< const mesh::MeshAsset > meshAsset = pipelineBuilder->getObjectReadOnly< mesh::MeshAsset >(meshComponent->getMesh());
			if (meshAsset)
			{
				merge->addVisualMesh(
					meshAsset,
					TprefabInv * entityData->getTransform()
				);
			}
			else
				log::warning << L"Skipped visual mesh of \"" << entityData->getName() << L"\"; unable to read visual mesh asset." << Endl;
		}

		const physics::RigidBodyComponentData* rigidBodyComponent = entityData->getComponent< physics::RigidBodyComponentData >();
		if (rigidBodyComponent)
		{
			const physics::BodyDesc* bodyDesc = rigidBodyComponent->getBodyDesc();
			if (bodyDesc)
			{
				const physics::MeshShapeDesc* meshShapeDesc = dynamic_type_cast< const physics::MeshShapeDesc* >(bodyDesc->getShape());
				if (meshShapeDesc)
				{
					Ref< const physics::MeshAsset > meshShapeAsset = pipelineBuilder->getObjectReadOnly< physics::MeshAsset >(meshShapeDesc->getMesh());
					if (meshShapeAsset)
					{
						merge->addShapeMesh(
							meshShapeAsset,
							TprefabInv * entityData->getTransform()
						);

						shapeCollisionGroup.insert(meshShapeDesc->getCollisionGroup().begin(), meshShapeDesc->getCollisionGroup().end());
						shapeCollisionMask.insert(meshShapeDesc->getCollisionMask().begin(), meshShapeDesc->getCollisionMask().end());
					}
					else
						log::warning << L"Skipped physics mesh of \"" << entityData->getName() << L"\"; unable to read collision mesh asset." << Endl;
				}
			}
		}
	}

	// Create our output entity which will only contain the merged meshes.
	Ref< world::EntityData > outputEntityData = new world::EntityData();
	outputEntityData->setName(prefabEntityData->getName());
	outputEntityData->setTransform(prefabEntityData->getTransform());

	// Merge visual meshes.
	if (!merge->getVisualMeshes().empty())
	{
		Ref< model::Model > mergedModel = new model::Model();
		std::map< std::wstring, Guid > mergedMaterialShaders;
		std::map< std::wstring, Guid > mergedMaterialTextures;
		std::map< std::wstring, Ref< const model::Model > > modelMap;

		uint32_t vertexCount = 0;
		uint32_t polygonCount = 0;

		for (const auto& visualMesh : merge->getVisualMeshes())
		{
			Ref< const mesh::MeshAsset > meshAsset = visualMesh.meshAsset;
			T_ASSERT(meshAsset);

			// Insert custom material shaders.
			for (const auto materialShader : meshAsset->getMaterialShaders())
			{
				const auto it = mergedMaterialShaders.find(materialShader.first);
				if (it != mergedMaterialShaders.end() && it->second != materialShader.second)
					log::warning << L"Different shaders on material with same name \"" << materialShader.first << L"\"; not allowed in prefab." << Endl;

				mergedMaterialShaders[materialShader.first] = materialShader.second;
			}

			// Insert material textures.
			for (const auto materialTexture : meshAsset->getMaterialTextures())
			{
				const auto it = mergedMaterialTextures.find(materialTexture.first);
				if (it != mergedMaterialTextures.end() && it->second != materialTexture.second)
					log::warning << L"Different textures on material with same name \"" << materialTexture.first << L"\"; not allowed in prefab." << Endl;

				mergedMaterialTextures[materialTexture.first] = materialTexture.second;
			}

			uint32_t currentVertexCount = mergedModel->getVertexCount();
			uint32_t currentPolygonCount = mergedModel->getPolygonCount();

			auto it = modelMap.find(meshAsset->getFileName().getOriginal());
			if (it != modelMap.end())
			{
				model::MergeModel(*(it->second), visualMesh.transform, m_visualMeshSnap).apply(*mergedModel);
				vertexCount += it->second->getVertexCount();
				polygonCount += it->second->getPolygonCount();
			}
			else
			{
				Path filePath = FileSystem::getInstance().getAbsolutePath(Path(m_assetPath) + meshAsset->getFileName());
				Ref< model::Model > partModel = modelCache.get(filePath, meshAsset->getImportFilter());
				if (!partModel)
				{
					log::warning << L"Unable to read model \"" << meshAsset->getFileName().getOriginal() << L"\"" << Endl;
					continue;
				}

				partModel->clear( model::Model::CfColors | model::Model::CfJoints );

				model::CleanDuplicates(0.01f).apply(*partModel);
				model::MergeModel(*partModel, visualMesh.transform, m_visualMeshSnap).apply(*mergedModel);

				vertexCount += partModel->getVertexCount();
				polygonCount += partModel->getPolygonCount();
				modelMap[meshAsset->getFileName().getOriginal()] = partModel;
			}
		}

		log::info << L"Output visual model ('original' to 'merged'):" << Endl;
		log::info << L"\t" << vertexCount << L" to " << mergedModel->getVertexCount() << L" vertices" << Endl;
		log::info << L"\t" << polygonCount << L" to " << mergedModel->getPolygonCount() << L" polygon(s)" << Endl;

		// Build output mesh from merged model.
		Ref< mesh::MeshAsset > mergedMeshAsset = new mesh::MeshAsset();
		mergedMeshAsset->setMeshType(prefabEntityData->partitionMesh() ? mesh::MeshAsset::MtPartition : mesh::MeshAsset::MtStatic);
		mergedMeshAsset->setMaterialShaders(mergedMaterialShaders);
		mergedMeshAsset->setMaterialTextures(mergedMaterialTextures);

		pipelineBuilder->buildOutput(
			sourceInstance,
			mergedMeshAsset,
			outputRenderMeshPath,
			outputRenderMeshGuid,
			mergedModel
		);

		// Replace mesh component referencing our merged mesh.
		outputEntityData->setComponent(new mesh::MeshComponentData(
			resource::Id< mesh::IMesh >(outputRenderMeshGuid)
		));
	}

	// Merge physics meshes.
	if (!merge->getShapeMeshes().empty())
	{
		Ref< model::Model > mergedModel = new model::Model();
		std::map< std::wstring, Ref< const model::Model > > modelMap;

		uint32_t vertexCount = 0;
		uint32_t polygonCount = 0;

		for (const auto& shapeMesh : merge->getShapeMeshes())
		{
			Ref< const physics::MeshAsset > meshShapeAsset = shapeMesh.meshAsset;
			T_ASSERT(meshShapeAsset);

			const auto it = modelMap.find(meshShapeAsset->getFileName().getOriginal());
			if (it != modelMap.end())
			{
				model::MergeModel(*(it->second), shapeMesh.transform, m_collisionMeshSnap).apply(*mergedModel);
			}
			else
			{
				Path filePath = FileSystem::getInstance().getAbsolutePath(Path(m_assetPath) + meshShapeAsset->getFileName());
				Ref< model::Model > partModel = modelCache.get(filePath, L"");
				if (!partModel)
				{
					log::warning << L"Unable to read model \"" << meshShapeAsset->getFileName().getOriginal() << L"\"." << Endl;
					continue;
				}

				partModel->clear(model::Model::CfMaterials | model::Model::CfColors | model::Model::CfNormals | model::Model::CfTexCoords | model::Model::CfJoints);

				model::CleanDuplicates(m_collisionMeshSnap).apply(*partModel);
				model::CleanDegenerate().apply(*partModel);

				//if (m_mergeCoplanar)
				//	model::MergeCoplanarAdjacents().apply(*partModel);

				model::MergeModel(*partModel, shapeMesh.transform, m_collisionMeshSnap).apply(*mergedModel);

				modelMap[meshShapeAsset->getFileName().getOriginal()] = partModel;
			}
		}

		// Collapse coplanar adjacent polygons.
		//if (m_mergeCoplanar)
		//	model::MergeCoplanarAdjacents().apply(*mergedModel);

		log::info << L"Output physics model ('original' to 'merged'):" << Endl;
		log::info << L"\t" << vertexCount << L" to " << mergedModel->getVertexCount() << L" vertices" << Endl;
		log::info << L"\t" << polygonCount << L" to " << mergedModel->getPolygonCount() << L" polygon(s)" << Endl;

		Ref< physics::MeshAsset > mergedMeshAsset = new physics::MeshAsset();
		mergedMeshAsset->setMargin(0.0f);
		mergedMeshAsset->setCalculateConvexHull(false);

		pipelineBuilder->buildOutput(
			sourceInstance,
			mergedMeshAsset,
			outputCollisionShapePath,
			outputCollisionShapeGuid,
			mergedModel
		);

		// Replace mesh component referencing our merged physics mesh.
		Ref< physics::MeshShapeDesc > outputShapeDesc = new physics::MeshShapeDesc();
		outputShapeDesc->setMesh(resource::Id< physics::Mesh >(outputCollisionShapeGuid));
		outputShapeDesc->setCollisionGroup(shapeCollisionGroup);
		outputShapeDesc->setCollisionMask(shapeCollisionMask);

		Ref< physics::StaticBodyDesc > outputBodyDesc = new physics::StaticBodyDesc();
		outputBodyDesc->setShape(outputShapeDesc);

		outputEntityData->setComponent(new physics::RigidBodyComponentData(
			outputBodyDesc
		));
	}

	return outputEntityData;
}

	}
}
