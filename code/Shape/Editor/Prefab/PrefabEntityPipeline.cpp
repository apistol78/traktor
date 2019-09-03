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
#include "Model/ModelFormat.h"
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
#include "World/Entity/ComponentEntityData.h"
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

void collectComponentEntities(const ISerializable* object, RefArray< world::ComponentEntityData >& outComponentEntities)
{
	Ref< Reflection > reflection = Reflection::create(object);

	RefArray< ReflectionMember > objectMembers;
	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);

	while (!objectMembers.empty())
	{
		Ref< RfmObject > objectMember = checked_type_cast< RfmObject*, false >(objectMembers.front());
		objectMembers.pop_front();

		if (world::ComponentEntityData* componentEntityData = dynamic_type_cast< world::ComponentEntityData* >(objectMember->get()))
		{
			outComponentEntities.push_back(componentEntityData);
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
	m_visualMeshSnap = settings->getProperty< float >(L"PrefabPipeline.VisualMeshSnap", 0.01f);
	m_collisionMeshSnap = settings->getProperty< float >(L"PrefabPipeline.CollisionMeshSnap", 0.01f);
	m_mergeCoplanar = settings->getProperty< bool >(L"PrefabPipeline.MergeCoplanar", true);
	m_usedGuids.clear();
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
	const PrefabEntityData* sourcePrefabEntityData = mandatory_non_null_type_cast< const PrefabEntityData* >(sourceAsset);
	
	// Get output guids so we can verify integrity early.
	Guid outputRenderMeshGuid = sourcePrefabEntityData->getOutputGuid(0);
	Guid outputCollisionShapeGuid = sourcePrefabEntityData->getOutputGuid(1);

	if (m_usedGuids.find(outputRenderMeshGuid) != m_usedGuids.end())
	{
		log::error << L"PrefabEntityPipeline failed; Output guid 0 of prefab \"" << sourcePrefabEntityData->getName() << L"\" already used." << Endl;
		return false;
	}
	m_usedGuids.insert(outputRenderMeshGuid);

	if (m_usedGuids.find(outputCollisionShapeGuid) != m_usedGuids.end())
	{
		log::error << L"PrefabEntityPipeline failed; Output guid 1 of prefab \"" << sourcePrefabEntityData->getName() << L"\" already used." << Endl;
		return false;
	}
	m_usedGuids.insert(outputCollisionShapeGuid);

	return world::EntityPipeline::buildDependencies(
		pipelineDepends,
		sourceInstance,
		sourceAsset,
		outputPath,
		outputGuid
	);
}

Ref< ISerializable > PrefabEntityPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	Ref< PrefabEntityData > prefabEntityData = checked_type_cast< PrefabEntityData* >(resolveAllExternal(pipelineBuilder, sourceAsset));
	if (!prefabEntityData)
	{
		log::error << L"Prefab entity pipeline failed; Unable to resolve all external entities." << Endl;
		return nullptr;
	}

	Guid outputRenderMeshGuid = prefabEntityData->getOutputGuid(0);
	Guid outputCollisionShapeGuid = prefabEntityData->getOutputGuid(1);

	std::wstring outputRenderMeshPath = L"Generated/" + outputRenderMeshGuid.format();
	std::wstring outputCollisionShapePath = L"Generated/" + outputCollisionShapeGuid.format();

	// Get all component entities which contain visual and/or physics meshes.
	RefArray< world::ComponentEntityData > componentEntityDatas;
	collectComponentEntities(prefabEntityData, componentEntityDatas);

	Ref< PrefabMerge > merge = new PrefabMerge();

	std::set< resource::Id< physics::CollisionSpecification > > shapeCollisionGroup;
	std::set< resource::Id< physics::CollisionSpecification > > shapeCollisionMask;

	Transform Tprefab = prefabEntityData->getTransform();
	Transform TprefabInv = Tprefab.inverse();

	for (auto componentEntityData : componentEntityDatas)
	{
		const mesh::MeshComponentData* meshComponent = componentEntityData->getComponent< mesh::MeshComponentData >();
		if (meshComponent)
		{
			Ref< const mesh::MeshAsset > meshAsset = pipelineBuilder->getObjectReadOnly< mesh::MeshAsset >(meshComponent->getMesh());
			if (meshAsset)
			{
				merge->addVisualMesh(
					meshAsset,
					TprefabInv * componentEntityData->getTransform()
				);
			}
			else
				log::warning << L"Skipped visual mesh of \"" << componentEntityData->getName() << L"\"; unable to read visual mesh asset." << Endl;
		}

		const physics::RigidBodyComponentData* rigidBodyComponent = componentEntityData->getComponent< physics::RigidBodyComponentData >();
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
							TprefabInv * componentEntityData->getTransform()
						);

						shapeCollisionGroup.insert(meshShapeDesc->getCollisionGroup().begin(), meshShapeDesc->getCollisionGroup().end());
						shapeCollisionMask.insert(meshShapeDesc->getCollisionMask().begin(), meshShapeDesc->getCollisionMask().end());
					}
					else
						log::warning << L"Skipped physics mesh of \"" << componentEntityData->getName() << L"\"; unable to read collision mesh asset." << Endl;
				}
			}
		}
	}

	// Create our output entity which will only contain the merged meshes.
	Ref< world::ComponentEntityData > outputEntityData = new world::ComponentEntityData();
	outputEntityData->setName(prefabEntityData->getName());
	outputEntityData->setTransform(prefabEntityData->getTransform());

	// Merge visual meshes.
	if (!merge->getVisualMeshes().empty())
	{
		Ref< model::Model > mergedModel = new model::Model();
		std::map< std::wstring, Guid > mergedMaterialShaders;
		std::map< std::wstring, Guid > mergedMaterialTextures;
		std::map< std::wstring, Ref< const model::Model > > modelCache;

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

			std::map< std::wstring, Ref< const model::Model > >::const_iterator j = modelCache.find(meshAsset->getFileName().getOriginal());
			if (j != modelCache.end())
			{
				model::MergeModel(*(j->second), visualMesh.transform, m_visualMeshSnap).apply(*mergedModel);
				vertexCount += j->second->getVertexCount();
				polygonCount += j->second->getPolygonCount();
			}
			else
			{
				Ref< model::Model > partModel = model::ModelFormat::readAny(meshAsset->getFileName(), [&](const Path& p) {
					return pipelineBuilder->openFile(Path(m_assetPath), p.getOriginal());
				});
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
				modelCache[meshAsset->getFileName().getOriginal()] = partModel;
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
		std::map< std::wstring, Ref< const model::Model > > modelCache;

		uint32_t vertexCount = 0;
		uint32_t polygonCount = 0;

		for (const auto& shapeMesh : merge->getShapeMeshes())
		{
			Ref< const physics::MeshAsset > meshShapeAsset = shapeMesh.meshAsset;
			T_ASSERT(meshShapeAsset);

			const auto it = modelCache.find(meshShapeAsset->getFileName().getOriginal());
			if (it != modelCache.end())
			{
				model::MergeModel(*(it->second), shapeMesh.transform, m_collisionMeshSnap).apply(*mergedModel);
			}
			else
			{
				Ref< model::Model > partModel = model::ModelFormat::readAny(meshShapeAsset->getFileName(), [&](const Path& p) {
					return pipelineBuilder->openFile(Path(m_assetPath), p.getOriginal());
				});
				if (!partModel)
				{
					log::warning << L"Unable to read model \"" << meshShapeAsset->getFileName().getOriginal() << L"\"" << Endl;
					continue;
				}

				partModel->clear(model::Model::CfMaterials | model::Model::CfColors | model::Model::CfNormals | model::Model::CfTexCoords | model::Model::CfJoints);

				model::CleanDuplicates(m_collisionMeshSnap).apply(*partModel);
				model::CleanDegenerate().apply(*partModel);

				if (m_mergeCoplanar)
					model::MergeCoplanarAdjacents().apply(*partModel);

				model::MergeModel(*partModel, shapeMesh.transform, m_collisionMeshSnap).apply(*mergedModel);

				modelCache[meshShapeAsset->getFileName().getOriginal()] = partModel;
			}
		}

		// Collapse coplanar adjacent polygons.
		if (m_mergeCoplanar)
			model::MergeCoplanarAdjacents().apply(*mergedModel);

		log::info << L"Output physics model ('original' to 'merged'):" << Endl;
		log::info << L"\t" << vertexCount << L" to " << mergedModel->getVertexCount() << L" vertices" << Endl;
		log::info << L"\t" << polygonCount << L" to " << mergedModel->getPolygonCount() << L" polygon(s)" << Endl;

		Ref< physics::MeshAsset > mergedMeshAsset = new physics::MeshAsset();
		mergedMeshAsset->setMargin(0.0f);
		mergedMeshAsset->setCalculateConvexHull(false);

		pipelineBuilder->buildOutput(
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
