#include "Runtime/Editor/Prefab/PrefabMerge.h"
#include "Runtime/Editor/Prefab/PrefabMergePipeline.h"
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/ModelAdjacency.h"
#include "Model/ModelFormat.h"
#include "Model/Operations/Boolean.h"
#include "Model/Operations/CleanDegenerate.h"
#include "Model/Operations/CleanDuplicates.h"
#include "Model/Operations/MergeCoplanarAdjacents.h"
#include "Model/Operations/MergeModel.h"
#include "Model/Operations/Quantize.h"
#include "Model/Operations/Transform.h"
#include "Physics/Editor/MeshAsset.h"

namespace traktor
{
	namespace runtime
	{
		namespace
		{

bool isModelClosed(const model::Model* model)
{
	model::ModelAdjacency adjacency(model, model::ModelAdjacency::MdByPosition);

	// All edges must have exactly one neighbor in order for the mesh to be closed.
	uint32_t edgeCount = adjacency.getEdgeCount();
	for (uint32_t i = 0; i < edgeCount; ++i)
	{
		if (adjacency.getSharedEdgeCount(i) != 1)
			return false;
	}

	return true;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.PrefabMergePipeline", 0, PrefabMergePipeline, editor::IPipeline)

PrefabMergePipeline::PrefabMergePipeline()
:	m_visualMeshSnap(0.01f)
,	m_collisionMeshSnap(0.01f)
,	m_mergeCoplanar(true)
{
}

bool PrefabMergePipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	m_visualMeshSnap = settings->getProperty< float >(L"PrefabPipeline.VisualMeshSnap", 0.01f);
	m_collisionMeshSnap = settings->getProperty< float >(L"PrefabPipeline.CollisionMeshSnap", 0.01f);
	m_mergeCoplanar = settings->getProperty< bool >(L"PrefabPipeline.MergeCoplanar", true);
	return true;
}

void PrefabMergePipeline::destroy()
{
}

TypeInfoSet PrefabMergePipeline::getAssetTypes() const
{
	return makeTypeInfoSet< PrefabMerge >();
}

bool PrefabMergePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const PrefabMerge* merge = mandatory_non_null_type_cast< const PrefabMerge* >(sourceAsset);

	for (const auto& visualMesh : merge->getVisualMeshes())
		pipelineDepends->addDependency(visualMesh.meshAsset);

	for (const auto& shapeMesh : merge->getShapeMeshes())
		pipelineDepends->addDependency(shapeMesh.meshAsset);

	pipelineDepends->addDependency< mesh::MeshAsset >();
	pipelineDepends->addDependency< physics::MeshAsset >();
	return true;
}

bool PrefabMergePipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::IPipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	const PrefabMerge* merge = mandatory_non_null_type_cast< const PrefabMerge* >(sourceAsset);

	uint32_t vertexCount = 0;
	uint32_t polygonCount = 0;

	// Merge visual models.
	if (!merge->getVisualMeshes().empty())
	{
		Ref< model::Model > mergedModel = new model::Model();
		std::map< std::wstring, Guid > mergedMaterialShaders;
		std::map< std::wstring, Guid > mergedMaterialTextures;
		std::map< std::wstring, Ref< const model::Model > > modelCache;

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
		mergedMeshAsset->setMeshType(merge->partitionMesh() ? mesh::MeshAsset::MtPartition : mesh::MeshAsset::MtStatic);
		mergedMeshAsset->setMaterialShaders(mergedMaterialShaders);
		mergedMeshAsset->setMaterialTextures(mergedMaterialTextures);

		pipelineBuilder->buildOutput(
			mergedMeshAsset,
			outputPath,
			outputGuid,
			mergedModel
		);
	}

	// Merge collision models.
	if (!merge->getShapeMeshes().empty())
	{
		Ref< model::Model > mergedModel = new model::Model();
		std::map< std::wstring, Ref< const model::Model > > modelCache;

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
					model::MergeCoplanarAdjacents(true).apply(*partModel);

				if (!isModelClosed(partModel))
					log::warning << L"Prefab collision model \"" << meshShapeAsset->getFileName().getOriginal() << L"\" is not closed!" << Endl;

				model::MergeModel(*partModel, shapeMesh.transform, m_collisionMeshSnap).apply(*mergedModel);

				modelCache[meshShapeAsset->getFileName().getOriginal()] = partModel;
			}
		}

		// Collapse coplanar adjacent polygons.
		if (m_mergeCoplanar)
			model::MergeCoplanarAdjacents(true).apply(*mergedModel);

		log::info << L"Output collision model ('original' to 'merged'):" << Endl;
		log::info << L"\t" << vertexCount << L" to " << mergedModel->getVertexCount() << L" vertices" << Endl;
		log::info << L"\t" << polygonCount << L" to " << mergedModel->getPolygonCount() << L" polygon(s)" << Endl;

		Ref< physics::MeshAsset > mergedMeshAsset = new physics::MeshAsset();
		mergedMeshAsset->setCalculateConvexHull(false);

		pipelineBuilder->buildOutput(
			mergedMeshAsset,
			outputPath,
			outputGuid,
			mergedModel
		);
	}

	return true;
}

Ref< ISerializable > PrefabMergePipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	T_FATAL_ERROR;
	return nullptr;
}

	}
}
