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
#include "Core/Misc/String.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyObject.h"
#include "Core/Settings/PropertyString.h"
#include "Drawing/Image.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Editor/MaterialShaderGenerator.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/ModelCache.h"
#include "Model/ModelFormat.h"
#include "Model/Operations/CleanDuplicates.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/StaticBodyDesc.h"
#include "Physics/Editor/MeshAsset.h"
#include "Physics/World/RigidBodyComponentData.h"
#include "Render/Shader.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Texture/TextureOutput.h"
#include "Shape/Editor/Spline/CloneShapeLayerData.h"
#include "Shape/Editor/Spline/ControlPointComponentData.h"
#include "Shape/Editor/Spline/ExtrudeShapeLayerData.h"
#include "Shape/Editor/Bake/Embree/SplitModel.h"
#include "Shape/Editor/Spline/SplineComponentData.h"
#include "Shape/Editor/Spline/SplineEntityPipeline.h"
#include "Shape/Editor/Spline/SplineEntityReplicator.h"
#include "World/EntityData.h"
#include "World/Entity/GroupComponentData.h"
#include "World/Entity/PathComponentData.h"

#include "Render/Editor/Shader/FragmentLinker.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphHash.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphOptimizer.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphStatic.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphTechniques.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphValidator.h"

namespace traktor::shape
{
	namespace
	{

const resource::Id< render::Shader > c_defaultShader(Guid(L"{F01DE7F1-64CE-4613-9A17-899B44D5414E}"));

class FragmentReaderAdapter : public render::FragmentLinker::IFragmentReader
{
public:
	explicit FragmentReaderAdapter(editor::IPipelineBuilder* pipelineBuilder)
	:	m_pipelineBuilder(pipelineBuilder)
	{
	}

	virtual Ref< const render::ShaderGraph > read(const Guid& fragmentGuid) const
	{
		return m_pipelineBuilder->getObjectReadOnly< render::ShaderGraph >(fragmentGuid);
	}

private:
	Ref< editor::IPipelineBuilder > m_pipelineBuilder;
};

AlignedVector< Guid > gatherMeshIds(const ISerializable* sourceAsset)
{
	AlignedVector< Guid > meshIds;
	if (auto cloneShapeLayerData = dynamic_type_cast< const CloneShapeLayerData* >(sourceAsset))
		meshIds.push_back(cloneShapeLayerData->getMesh());
	else if (auto extrudeShapeLayerData = dynamic_type_cast< const ExtrudeShapeLayerData* >(sourceAsset))
	{
		meshIds.push_back(extrudeShapeLayerData->getMeshStart());
		meshIds.push_back(extrudeShapeLayerData->getMeshRepeat());
		meshIds.push_back(extrudeShapeLayerData->getMeshEnd());
	}
	return meshIds;
}

bool buildEmbeddedTexture(editor::IPipelineBuilder* pipelineBuilder, model::Material::Map& map, bool normalMap)
{
	if (map.image == nullptr || map.texture.isNotNull())
		return true;

	const uint32_t hash = DeepHash(map.image).get();
	const Guid outputGuid = Guid(L"{6E6346F6-4665-42DC-BE69-58D8B7A475E4}").permutation(hash);

	Ref< render::TextureOutput > output = new render::TextureOutput();
	output->m_textureType = render::Tt2D;
	output->m_normalMap = normalMap;
	output->m_assumeLinearGamma = normalMap;

	if (!pipelineBuilder->buildAdHocOutput(
		output,
		outputGuid,
		map.image
	))
		return false;

	map.texture = outputGuid;
	return true;
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.SplineEntityPipeline", 10, SplineEntityPipeline, world::EntityPipeline)

bool SplineEntityPipeline::create(const editor::IPipelineSettings* settings)
{
	if (!world::EntityPipeline::create(settings))
		return false;

	m_assetPath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.AssetPath", L"");
	m_modelCachePath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.ModelCache.Path", L"");
	m_platform = settings->getPropertyIncludeHash< std::wstring >(L"ShaderPipeline.Platform");
	m_targetEditor = settings->getPropertyIncludeHash< bool >(L"Pipeline.TargetEditor", false);
	
	m_replicator = new SplineEntityReplicator();
	m_replicator->create(settings);

	return true;
}

TypeInfoSet SplineEntityPipeline::getAssetTypes() const
{
	return makeTypeInfoSet<
		CloneShapeLayerData,
		ControlPointComponentData,
		ExtrudeShapeLayerData,
		SplineComponentData
	>();
}

bool SplineEntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	AlignedVector< Guid > meshIds;

	if (auto splineComponentData = dynamic_type_cast< const SplineComponentData* >(sourceAsset))
	{
		for (auto id : splineComponentData->getCollisionGroup())
			pipelineDepends->addDependency(id, editor::PdfBuild | editor::PdfResource);
		for (auto id : splineComponentData->getCollisionMask())
			pipelineDepends->addDependency(id, editor::PdfBuild | editor::PdfResource);	
	}

	for (const auto& meshId : gatherMeshIds(sourceAsset))
	{
		const uint32_t flags = m_targetEditor ? editor::PdfBuild | editor::PdfUse : editor::PdfUse;
		pipelineDepends->addDependency(meshId, flags);
	}

	if (m_targetEditor)
		pipelineDepends->addDependency(c_defaultShader, editor::PdfBuild | editor::PdfResource);

	return world::EntityPipeline::buildDependencies(pipelineDepends, sourceInstance, sourceAsset, outputPath, outputGuid);
}

Ref< ISerializable > SplineEntityPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	if (m_targetEditor)
	{
		mesh::MaterialShaderGenerator materialGenerator(
			[&](const Guid& fragmentId) { return pipelineBuilder->getObjectReadOnly< render::ShaderGraph >(fragmentId); }
		);

		for (const auto& meshId : gatherMeshIds(sourceAsset))
		{
			Ref< const mesh::MeshAsset > meshAsset = pipelineBuilder->getObjectReadOnly< mesh::MeshAsset >(meshId);
			if (!meshAsset)
				continue;

			// Read model specified by mesh asset.
			const Path filePath = FileSystem::getInstance().getAbsolutePath(Path(m_assetPath) + meshAsset->getFileName());
			Ref< model::Model > model = model::ModelCache::getInstance().getMutable(m_modelCachePath, filePath, meshAsset->getImportFilter());
			if (!model)
				continue;

			for (uint32_t i = 0; i < model->getMaterialCount(); ++i)
			{
				model::Material material = model->getMaterial(i);

				const uint32_t materialHash = DeepHash(&material).get();
				const Guid outputGuid = Guid(L"{8BB018D2-7AAC-4F9D-A5A4-DE396604862C}").permutation(materialHash);

				buildEmbeddedTexture(pipelineBuilder, material.getDiffuseMap(), false);
				buildEmbeddedTexture(pipelineBuilder, material.getSpecularMap(), false);
				buildEmbeddedTexture(pipelineBuilder, material.getRoughnessMap(), false);
				buildEmbeddedTexture(pipelineBuilder, material.getMetalnessMap(), false);
				buildEmbeddedTexture(pipelineBuilder, material.getTransparencyMap(), false);
				buildEmbeddedTexture(pipelineBuilder, material.getEmissiveMap(), false);
				buildEmbeddedTexture(pipelineBuilder, material.getReflectiveMap(), false);
				buildEmbeddedTexture(pipelineBuilder, material.getNormalMap(), true);

				Ref< const render::ShaderGraph > meshSurfaceShaderGraph = materialGenerator.generateSurface(
					*model,
					material,
					false,
					true
				);

				Ref< const render::ShaderGraph > materialShaderGraph = materialGenerator.generateMesh(
					*model,
					material,
					meshSurfaceShaderGraph,
					Guid(L"{14AE48E1-723D-0944-821C-4B73AC942437}")
				);

				// Resolve all variables.
				materialShaderGraph = render::ShaderGraphStatic(materialShaderGraph, outputGuid).getVariableResolved();
				if (!materialShaderGraph)
				{
					log::error << L"SplineEntityPipeline failed; unable to resolve variables." << Endl;
					return nullptr;
				}

				// Link shader fragments.
				FragmentReaderAdapter fragmentReader(pipelineBuilder);
				materialShaderGraph = render::FragmentLinker(fragmentReader).resolve(materialShaderGraph, true);
				if (!materialShaderGraph)
				{
					log::error << L"SplineEntityPipeline failed; unable to link shader fragments." << Endl;
					return nullptr;
				}

				// Resolve all bundles.
				materialShaderGraph = render::ShaderGraphStatic(materialShaderGraph, outputGuid).getBundleResolved();
				if (!materialShaderGraph)
				{
					log::error << L"SplineEntityPipeline failed; unable to resolve bundles." << Endl;
					return nullptr;
				}

				// Get connected permutation.
				materialShaderGraph = render::ShaderGraphStatic(materialShaderGraph, outputGuid).getConnectedPermutation();
				if (!materialShaderGraph)
				{
					log::error << L"SplineEntityPipeline failed; unable to freeze connected conditionals." << Endl;
					return nullptr;
				}

				// Extract platform permutation.
				materialShaderGraph = render::ShaderGraphStatic(materialShaderGraph, outputGuid).getPlatformPermutation(m_platform);
				if (!materialShaderGraph)
				{
					log::error << L"SplineEntityPipeline failed; unable to get platform \"" << m_platform << L"\" permutation." << Endl;
					return nullptr;
				}

				// Extract renderer permutation.
				const wchar_t* rendererSignature = L"Vulkan"; // programCompiler->getRendererSignature();
				materialShaderGraph = render::ShaderGraphStatic(materialShaderGraph, outputGuid).getRendererPermutation(rendererSignature);
				if (!materialShaderGraph)
				{
					log::error << L"SplineEntityPipeline failed; unable to get renderer permutation." << Endl;
					return nullptr;
				}

				// Freeze types, get typed permutation.
				materialShaderGraph = render::ShaderGraphStatic(materialShaderGraph, outputGuid).getTypePermutation();
				if (!materialShaderGraph)
				{
					log::error << L"SplineEntityPipeline failed; unable to freeze types." << Endl;
					return nullptr;
				}

				// Cleanup unused branches.
				materialShaderGraph = render::ShaderGraphOptimizer(materialShaderGraph).removeUnusedBranches(true);
				if (!materialShaderGraph)
				{
					log::error << L"SplineEntityPipeline failed; unable to cleanup shader." << Endl;
					return nullptr;
				}

				pipelineBuilder->buildAdHocOutput(materialShaderGraph, outputGuid);
			}
		}

		// In editor we generate spline geometry dynamically thus
		// not necessary to explicitly build mesh when building for editor.
		return world::EntityPipeline::buildProduct(pipelineBuilder, sourceInstance, sourceAsset, buildParams);
	}

	if (auto splineComponentData = dynamic_type_cast< const SplineComponentData* >(sourceAsset))
	{
		const world::EntityData* owner = mandatory_non_null_type_cast< const world::EntityData* >(buildParams);

		const Guid& entityId = owner->getId();
		if (entityId.isNull())
		{
			log::error << L"No ID on spline entity, need to ID to generate runtime meshes." << Endl;
			return nullptr;
		}

		Ref< model::Model > visualModel = m_replicator->createModel(pipelineBuilder, owner, splineComponentData, world::IEntityReplicator::Usage::Visual);
		if (!visualModel)
		{
			log::warning << L"Unable to create visual model from spline \"" << owner->getName() << L"\"." << Endl;
			return nullptr;
		}

		Ref< model::Model > collisionModel = m_replicator->createModel(pipelineBuilder, owner, splineComponentData, world::IEntityReplicator::Usage::Collision);
		if (!collisionModel)
		{
			log::warning << L"Unable to create collision model from spline \"" << owner->getName() << L"\"." << Endl;
			return nullptr;
		}

		Ref< world::EntityData > replacementEntityData = new world::EntityData();
		replacementEntityData->setId(owner->getId());
		replacementEntityData->setName(owner->getName());
		replacementEntityData->setTransform(owner->getTransform());

		// Add path component so runtime can query spline.
		{
			TransformPath path;

			// Get group component.
			auto group = owner->getComponent< world::GroupComponentData >();
			if (!group)
			{
				log::error << L"Invalid spline; no control points found." << Endl;
				return nullptr;
			}

			// Count number of control points as we need to estimate fraction of each.
			int32_t controlPointCount = 0;
			for (auto entityData : group->getEntityData())
			{
				for (auto componentData : entityData->getComponents())
				{
					if (is_a< ControlPointComponentData >(componentData))
						controlPointCount++;
				}
			}
			if (controlPointCount <= 0)
			{
				log::error << L"Invalid spline; no control points found." << Endl;
				return nullptr;
			}

			// Create transformation path.
			int32_t controlPointIndex = 0;
			for (auto entityData : group->getEntityData())
			{
				auto controlPointData = entityData->getComponent< ControlPointComponentData >();
				if (!controlPointData)
					continue;

				const Transform T = entityData->getTransform();

				TransformPath::Key k;
				k.T = (float)controlPointIndex / (controlPointCount - 1);
				k.position = T.translation();
				k.orientation = T.rotation().toEulerAngles();
				k.values[0] = controlPointData->getScale();
				path.insert(k);

				++controlPointIndex;
			}

			path = path.geometricNormalized(splineComponentData->isClosed());

			replacementEntityData->setComponent(new world::PathComponentData(path));
		}

		const Guid baseOutputMeshId = owner->getId();
		const Guid baseShapeId = owner->getId().permutation(10000);
		log::info << L"Spline " << owner->getName() << L", mesh ID " << baseOutputMeshId.format() << L", shape ID " << baseShapeId.format() << Endl;

		{
			RefArray< model::Model > splitVisualModels;
			splitModel(visualModel, splitVisualModels);
			log::info << L"Split model into " << (int32_t)splitVisualModels.size() << L" pieces." << Endl;

			Ref< const mesh::MeshAsset > meshAsset = dynamic_type_cast< const mesh::MeshAsset* >(
				visualModel->getProperty< ISerializable >(type_name< mesh::MeshAsset >())
			);

			Guid outputMeshId = baseOutputMeshId;

			Ref< world::GroupComponentData > groupData = new world::GroupComponentData();
			for (auto splitVisualModel : splitVisualModels)
			{
				// Ensure each split doesn't contain more than what is used.
				model::CleanDuplicates(0.01f).apply(*splitVisualModel);

				// Create split output entity data.
				Ref< world::EntityData > visualEntityData = new world::EntityData();
				visualEntityData->setId(outputMeshId.permutation(10000));
				visualEntityData->setTransform(replacementEntityData->getTransform());
				groupData->addEntityData(visualEntityData);

				// Create static mesh component.
				visualEntityData->setComponent(new mesh::MeshComponentData(resource::Id< mesh::IMesh >(outputMeshId)));

				// Build output mesh from merged model.
				Ref< mesh::MeshAsset > outputMeshAsset = new mesh::MeshAsset();
				if (meshAsset)
				{
					outputMeshAsset->setMaterialShaders(meshAsset->getMaterialShaders());
					outputMeshAsset->setMaterialTextures(meshAsset->getMaterialTextures());
				}

				pipelineBuilder->buildAdHocOutput(
					outputMeshAsset,
					outputMeshId,
					splitVisualModel
				);

				outputMeshId.permutate();
			}

			replacementEntityData->setComponent(groupData);
		}

		{
			const Guid outputShapeId = baseShapeId;

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

	// All other components should be discarded.
	return nullptr;
}

}
