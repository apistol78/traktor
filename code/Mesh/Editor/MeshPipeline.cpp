/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/Editor/MeshPipeline.h"

#include "Compress/Lzf/DeflateStreamLzf.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Format.h"
#include "Core/Misc/String.h"
#include "Core/Misc/WildCompare.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Core/Thread/Acquire.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Editor/Pipeline/PipelineProfiler.h"
#include "Mesh/Editor/Instance/InstanceMeshConverter.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Mesh/Editor/Skinned/SkinnedMeshConverter.h"
#include "Mesh/Editor/Static/StaticMeshConverter.h"
#include "Mesh/Editor/VertexShaderGenerator.h"
#include "Mesh/MeshResource.h"
#include "Model/Model.h"
#include "Model/ModelCache.h"
#include "Model/Operations/CalculateNormals.h"
#include "Model/Operations/CalculateTangents.h"
#include "Model/Operations/CullDistantFaces.h"
#include "Model/Operations/Reduce.h"
#include "Model/Operations/Transform.h"
#include "Render/Editor/IProgramCompiler.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphHash.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphOptimizer.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphStatic.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphTechniques.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphValidator.h"
#include "Render/Editor/Shader/External.h"
#include "Render/Editor/Shader/FragmentLinker.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ParameterLinker.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderGraphPreview.h"
#include "Render/Editor/Texture/TextureOutput.h"
#include "Render/Editor/Texture/TextureSet.h"
#include "World/Editor/Material/MaterialShaderGenerator.h"

#include <list>

namespace traktor::mesh
{
namespace
{

class FragmentReaderAdapter : public render::FragmentLinker::IFragmentReader
{
public:
	explicit FragmentReaderAdapter(editor::IPipelineBuilder* pipelineBuilder)
		: m_pipelineBuilder(pipelineBuilder)
	{
	}

	virtual Ref< const render::ShaderGraph > read(const Guid& fragmentGuid) const
	{
		Ref< const render::ShaderGraph > shaderGraph = m_pipelineBuilder->getObjectReadOnly< render::ShaderGraph >(fragmentGuid);
		if (!shaderGraph)
			return nullptr;

		// if (render::ShaderGraphValidator(shaderGraph, fragmentGuid).validateIntegrity())
		// 	return shaderGraph;
		// else
		// 	return nullptr;

		return shaderGraph;
	}

private:
	Ref< editor::IPipelineBuilder > m_pipelineBuilder;
};

bool haveVertexColors(const model::Model& model)
{
	for (uint32_t i = 0; i < model.getVertexCount(); ++i)
		if (model.getVertex(i).getColor() != model::c_InvalidIndex)
			return true;
	return false;
}

Guid getVertexShaderGuid(MeshAsset::MeshType meshType)
{
	switch (meshType)
	{
	case MeshAsset::MtInstance:
		return Guid(L"{A714A83F-8442-6F48-A2A7-6EFA95EB75F3}");

	case MeshAsset::MtSkinned:
		return Guid(L"{69A3CF2E-9B63-0440-9410-70AB4AE127CE}");

	case MeshAsset::MtStatic:
		return Guid(L"{14AE48E1-723D-0944-821C-4B73AC942437}");

	default:
		return Guid();
	}
}

bool buildEmbeddedTexture(editor::IPipelineBuilder* pipelineBuilder, model::Material::Map& map, bool normalMap)
{
	if (map.image == nullptr || map.texture.isNotNull())
		return true;

	pipelineBuilder->getProfiler()->begin(L"MeshPipeline buildEmbeddedTexture");

	const uint32_t hash = DeepHash(map.image).get();
	const Guid outputGuid = Guid(L"{6E6346F6-4665-42DC-BE69-58D8B7A475E4}").permutation(hash);

	Ref< render::TextureOutput > output = new render::TextureOutput();
	output->m_textureType = render::Tt2D;
	output->m_normalMap = normalMap;

	if (!pipelineBuilder->buildAdHocOutput(
			output,
			outputGuid,
			map.image))
	{
		pipelineBuilder->getProfiler()->end();
		return false;
	}

	map.texture = outputGuid;

	pipelineBuilder->getProfiler()->end();
	return true;
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.MeshPipeline", 60, MeshPipeline, editor::IPipeline)

MeshPipeline::MeshPipeline()
	: m_promoteHalf(false)
	, m_enableCustomShaders(true)
	, m_enableCustomTemplates(true)
	, m_editor(false)
{
}

bool MeshPipeline::create(const editor::IPipelineSettings* settings, db::Database* database)
{
	m_assetPath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.AssetPath");
	m_modelCachePath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.ModelCache.Path");
	m_promoteHalf = settings->getPropertyIncludeHash< bool >(L"MeshPipeline.PromoteHalf", false);
	m_enableCustomShaders = settings->getPropertyIncludeHash< bool >(L"MeshPipeline.EnableCustomShaders", true);
	m_enableCustomTemplates = settings->getPropertyIncludeHash< bool >(L"MeshPipeline.EnableCustomTemplates", true);
	m_includeOnlyTechniques = settings->getPropertyIncludeHash< SmallSet< std::wstring > >(L"ShaderPipeline.IncludeOnlyTechniques");
	m_programCompilerTypeName = settings->getPropertyIncludeHash< std::wstring >(L"ShaderPipeline.ProgramCompiler");
	m_platform = settings->getPropertyIncludeHash< std::wstring >(L"ShaderPipeline.Platform");
	m_editor = settings->getPropertyIncludeHash< bool >(L"Pipeline.TargetEditor", false);
	return true;
}

void MeshPipeline::destroy()
{
	m_programCompiler = nullptr;
}

TypeInfoSet MeshPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< MeshAsset >();
}

bool MeshPipeline::shouldCache() const
{
	return true;
}

uint32_t MeshPipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
}

bool MeshPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid) const
{
	Ref< const MeshAsset > asset = checked_type_cast< const MeshAsset* >(sourceAsset);
	T_ASSERT(asset);

	// Add dependency to texture pipeline since we ad-hoc build embedded textures.
	pipelineDepends->addDependency< render::TextureOutput >();

	if (!asset->getFileName().empty())
		pipelineDepends->addDependency(Path(m_assetPath), asset->getFileName().getOriginal());

	// Determine vertex shader guid.
	const Guid vertexShaderGuid = getVertexShaderGuid(asset->getMeshType());
	if (!vertexShaderGuid.isValid())
	{
		log::error << L"Mesh pipeline failed; unknown mesh asset type" << Endl;
		return false;
	}
	pipelineDepends->addDependency(vertexShaderGuid, editor::PdfUse);

	// Add dependencies to generator fragments.
	VertexShaderGenerator::addDependencies(pipelineDepends);
	world::MaterialShaderGenerator::addDependencies(pipelineDepends);

	// Add dependencies to "fixed" material shaders.
	if (m_enableCustomShaders)
		for (const auto& it : asset->getMaterialShaders())
			pipelineDepends->addDependency(it.second, editor::PdfUse);

	// Add dependencies to material textures.
	for (const auto& it : asset->getMaterialTextures())
		pipelineDepends->addDependency(it.second, editor::PdfBuild | editor::PdfResource);

	pipelineDepends->addDependency< render::ShaderGraph >();

	// Add dependencies from mesh subsystems.
	InstanceMeshConverter::addDependencies(pipelineDepends);
	SkinnedMeshConverter::addDependencies(pipelineDepends);
	return true;
}

bool MeshPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::PipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* /*sourceInstance*/,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t /*reason*/
) const
{
	// SmallMap< std::wstring, model::Material > materials;
	Ref< model::Model > model;

	Ref< render::IProgramCompiler > programCompiler = getProgramCompiler();
	if (!programCompiler)
		return false;

	const auto asset = mandatory_non_null_type_cast< const MeshAsset* >(sourceAsset);
	const auto& materialShaders = asset->getMaterialShaders();
	const auto& materialTextures = asset->getMaterialTextures();

	// Create mesh converter.
	Ref< IMeshConverter > converter;
	switch (asset->getMeshType())
	{
	case MeshAsset::MtInstance:
		converter = new InstanceMeshConverter();
		break;

	case MeshAsset::MtSkinned:
		converter = new SkinnedMeshConverter();
		break;

	case MeshAsset::MtStatic:
		converter = new StaticMeshConverter();
		break;

	default:
		log::error << L"Mesh pipeline failed; unknown mesh asset type." << Endl;
		return false;
	}

	// Create list of model operations we need to perform on model before converting it.
	RefArray< const model::IModelOperation > operations;

	if (asset->getReduce() < 1.0f)
		operations.push_back(new model::Reduce(asset->getReduce()));

	if (!converter->getOperations(asset, m_editor, operations))
	{
		log::error << L"Mesh pipeline failed; unable to create model operations." << Endl;
		return false;
	}

	// Scale model according to scale factor in asset.
	operations.push_back(new model::Transform(
		translate(asset->getOffset()) *
		scale(asset->getScaleFactor())));

	// Recalculate normals regardless if already exist in model.
	if (asset->getRenormalize())
	{
		operations.push_back(new model::CalculateNormals(true));
		operations.push_back(new model::CalculateTangents(true));
	}
	else
	{
		operations.push_back(new model::CalculateNormals(false));
		operations.push_back(new model::CalculateTangents(false));
	}

	// We allow models to be passed as build parameters in case models
	// are procedurally generated.
	if (buildParams)
	{
		// Create a mutable copy of model since we modify it.
		pipelineBuilder->getProfiler()->begin(L"MeshPipeline clone model");
		model = DeepClone(checked_type_cast< const ISerializable* >(buildParams)).create< model::Model >();
		pipelineBuilder->getProfiler()->end();
	}
	else
	{
		log::info << L"Loading model \"" << asset->getFileName().getFileName() << L"\"..." << Endl;

		// Load and prepare models through model cache.
		const Path filePath = FileSystem::getInstance().getAbsolutePath(Path(m_assetPath) + asset->getFileName());
		model = model::ModelCache::getInstance().getMutable(m_modelCachePath, filePath, asset->getImportFilter());
	}

	if (model == nullptr)
	{
		log::error << L"Mesh pipeline failed; unable to read source model (" << asset->getFileName().getOriginal() << L")." << Endl;
		return false;
	}

	if (model->getPolygonCount() == 0)
	{
		log::error << L"Mesh pipeline failed; no polygons in source model (" << asset->getFileName().getOriginal() << L")." << Endl;
		return false;
	}

	model->apply(operations);

	// Merge all materials into a single list (duplicates will be overridden).
	if (asset->getCenter())
	{
		const Aabb3 boundingBox = model->getBoundingBox();
		model->apply(model::Transform(translate(-boundingBox.getCenter())));
	}

	if (asset->getGrounded())
	{
		const Aabb3 boundingBox = model->getBoundingBox();
		model->apply(model::Transform(translate(Vector4(0.0f, -boundingBox.mn.y(), 0.0f))));
	}

	AlignedVector< model::Material >& modelMaterials = model->getMaterials();
	if (model->getMaterials().empty())
	{
		log::error << L"Mesh pipeline failed; no materials in source model(s)." << Endl;
		return false;
	}

	// Ensure materials doesn't contain texture references for materials with custom shaders.
	for (model::Material& m : modelMaterials)
	{
		const auto it = materialShaders.find(m.getName());
		if (it != materialShaders.end())
		{
			model::Material::Map maps[] = {
				m.getDiffuseMap(),
				m.getSpecularMap(),
				m.getRoughnessMap(),
				m.getMetalnessMap(),
				m.getTransparencyMap(),
				m.getEmissiveMap(),
				m.getReflectiveMap(),
				m.getNormalMap()
			};

			for (auto& map : maps)
			{
				map.texture = Guid::null;
				map.image = nullptr;
			}

			// Try to generate a preview texture from custom material shader; can be used as albedo in RT.
			Ref< const render::ShaderGraph > materialShaderGraph = pipelineBuilder->getSourceDatabase()->getObjectReadOnly< render::ShaderGraph >(it->second);
			if (materialShaderGraph)
			{
				Ref< drawing::Image > image = render::ShaderGraphPreview(m_assetPath, pipelineBuilder->getSourceDatabase()).generate(materialShaderGraph, 128, 128);
				if (image)
				{
					maps[0].image = image;
					buildEmbeddedTexture(pipelineBuilder, maps[0], false);
				}
			}

			m.setDiffuseMap(maps[0]);
			m.setSpecularMap(maps[1]);
			m.setRoughnessMap(maps[2]);
			m.setMetalnessMap(maps[3]);
			m.setTransparencyMap(maps[4]);
			m.setEmissiveMap(maps[5]);
			m.setReflectiveMap(maps[6]);
			m.setNormalMap(maps[7]);
		}
	}

	// Merge materials, set textures specified in MeshAsset into material maps.
	for (model::Material& m : modelMaterials)
	{
		const bool materialHaveCustomShader = (bool)(materialShaders.find(m.getName()) != materialShaders.end());
		if (!materialHaveCustomShader)
		{
			model::Material::Map maps[] = {
				m.getDiffuseMap(),
				m.getSpecularMap(),
				m.getRoughnessMap(),
				m.getMetalnessMap(),
				m.getTransparencyMap(),
				m.getEmissiveMap(),
				m.getReflectiveMap(),
				m.getNormalMap()
			};

			for (auto& map : maps)
			{
				auto it = materialTextures.find(map.name);
				if (it != materialTextures.end())
					map.texture = it->second;
			}

			m.setDiffuseMap(maps[0]);
			m.setSpecularMap(maps[1]);
			m.setRoughnessMap(maps[2]);
			m.setMetalnessMap(maps[3]);
			m.setTransparencyMap(maps[4]);
			m.setEmissiveMap(maps[5]);
			m.setReflectiveMap(maps[6]);
			m.setNormalMap(maps[7]);
		}
	}

	const Aabb3 boundingBox = model->getBoundingBox();
	const uint32_t polygonCount = model->getPolygonCount();

	// Build embedded textures and assign generated id;s to materials.
	for (auto& m : modelMaterials)
	{
		const bool materialHaveCustomShader = (bool)(materialShaders.find(m.getName()) != materialShaders.end());
		if (!materialHaveCustomShader)
		{
			buildEmbeddedTexture(pipelineBuilder, m.getDiffuseMap(), false);
			buildEmbeddedTexture(pipelineBuilder, m.getSpecularMap(), false);
			buildEmbeddedTexture(pipelineBuilder, m.getRoughnessMap(), false);
			buildEmbeddedTexture(pipelineBuilder, m.getMetalnessMap(), false);
			buildEmbeddedTexture(pipelineBuilder, m.getTransparencyMap(), false);
			buildEmbeddedTexture(pipelineBuilder, m.getEmissiveMap(), false);
			buildEmbeddedTexture(pipelineBuilder, m.getReflectiveMap(), false);
			buildEmbeddedTexture(pipelineBuilder, m.getNormalMap(), true);
		}
	}

	// Build materials.
	AlignedVector< render::VertexElement > vertexElements;
	uint32_t vertexElementOffset = 0;

	std::map< uint32_t, Ref< render::ShaderGraph > > materialTechniqueShaderGraphs;	   //< Collection of all material technique fragments; later merged into single shader.
	std::map< std::wstring, std::list< MeshMaterialTechnique > > materialTechniqueMap; //< Map from model material to technique fragments. ["Model material":["Default":hash0, "Depth":hash1, ...]]

	const Guid vertexShaderGuid = getVertexShaderGuid(asset->getMeshType());
	T_ASSERT(vertexShaderGuid.isValid());

	const Guid materialGuid = vertexShaderGuid.permutation(outputGuid);
	T_ASSERT(materialGuid.isValid());

	const auto fragmentReader = [&](const Guid& fragmentId) {
		return pipelineBuilder->getObjectReadOnly< render::ShaderGraph >(fragmentId);
	};

	VertexShaderGenerator vertexGenerator(fragmentReader);
	world::MaterialShaderGenerator materialGenerator(fragmentReader);

	const int32_t jointCount = model->getJointCount();
	const bool haveVertexColor = haveVertexColors(*model);

	for (const auto& m : modelMaterials)
	{
		const std::wstring& materialName = m.getName();

		Ref< render::ShaderGraph > materialShaderGraph;
		Guid materialShaderGraphId;

		pipelineBuilder->getProfiler()->begin(L"MeshPipeline generateSurface");
		Ref< const render::ShaderGraph > meshSurfaceShaderGraph = materialGenerator.generateSurface(
			m,
			haveVertexColor,
			asset->getDecalResponse());
		pipelineBuilder->getProfiler()->end();
		if (!meshSurfaceShaderGraph)
		{
			log::error << L"Mesh pipeline failed; unable to generate material surface shader \"" << materialName << L"\"." << Endl;
			return false;
		}

		auto it = materialShaders.find(materialName);
		if (
			m_enableCustomShaders &&
			it != materialShaders.end())
		{
			if (it->second.isNull())
			{
				log::info << L"Material \"" << materialName << L"\" disabled; skipped." << Endl;
				continue;
			}

			Ref< const render::ShaderGraph > customMeshSurfaceShaderGraph = pipelineBuilder->getObjectReadOnly< render::ShaderGraph >(it->second);
			if (!customMeshSurfaceShaderGraph)
			{
				log::error << L"Mesh pipeline failed; unable to read material surface shader \"" << materialName << L"\"." << Endl;
				return false;
			}

			// Combine generated surface shader with custom; this allows the custom surface shader
			// to access/pass through surface properties from the generated surface shader without having
			// to ensure all are set in the custom shader itself.

			pipelineBuilder->getProfiler()->begin(L"MeshPipeline combineSurface");
			customMeshSurfaceShaderGraph = materialGenerator.combineSurface(customMeshSurfaceShaderGraph, meshSurfaceShaderGraph);
			pipelineBuilder->getProfiler()->end();
			if (!customMeshSurfaceShaderGraph)
			{
				log::error << L"Mesh pipeline failed; unable to combine material surface shaders \"" << materialName << L"\"." << Endl;
				return false;
			}

			pipelineBuilder->getProfiler()->begin(L"MeshPipeline generateMesh");
			materialShaderGraph = vertexGenerator.generateMesh(
				*model,
				m,
				customMeshSurfaceShaderGraph,
				vertexShaderGuid);
			pipelineBuilder->getProfiler()->end();
			if (!materialShaderGraph)
			{
				log::error << L"Mesh pipeline failed; unable to generate material mesh shader \"" << materialName << L"\"." << Endl;
				return false;
			}

			materialShaderGraphId = it->second;
		}
		else
		{
			pipelineBuilder->getProfiler()->begin(L"MeshPipeline generateMesh");
			materialShaderGraph = vertexGenerator.generateMesh(
				*model,
				m,
				meshSurfaceShaderGraph,
				vertexShaderGuid);
			pipelineBuilder->getProfiler()->end();
			if (!materialShaderGraph)
			{
				log::error << L"Mesh pipeline failed; unable to generate material mesh shader \"" << materialName << L"\"." << Endl;
				return false;
			}
		}

		// Link shader fragments.
		pipelineBuilder->getProfiler()->begin(L"MeshPipeline link fragments");
		FragmentReaderAdapter fragmentReader(pipelineBuilder);
		materialShaderGraph = render::FragmentLinker(fragmentReader).resolve(materialShaderGraph, true);
		pipelineBuilder->getProfiler()->end();
		if (!materialShaderGraph)
		{
			log::error << L"MeshPipeline failed; unable to link shader fragments, material shader \"" << materialName << L"\"." << Endl;
			return false;
		}

		// Link parameter declarations.
		pipelineBuilder->getProfiler()->begin(L"MeshPipeline getTypePermutation");
		const auto parameterDeclarationReader = [&](const Guid& declarationId) -> render::ParameterLinker::named_decl_t {
			Ref< db::Instance > declarationInstance = pipelineBuilder->getSourceDatabase()->getInstance(declarationId);
			if (declarationInstance != nullptr)
				return { declarationInstance->getName(), declarationInstance->getObject() };
			else
				return { L"", nullptr };
		};
		if (!render::ParameterLinker(parameterDeclarationReader).resolve(materialShaderGraph))
		{
			log::error << L"MeshPipeline failed; unable to link parameters." << Endl;
			return false;
		}
		pipelineBuilder->getProfiler()->end();

		// Resolve all bundles.
		pipelineBuilder->getProfiler()->begin(L"MeshPipeline getBundleResolved");
		materialShaderGraph = render::ShaderGraphStatic(materialShaderGraph, materialShaderGraphId).getBundleResolved();
		pipelineBuilder->getProfiler()->end();
		if (!materialShaderGraph)
		{
			log::error << L"MeshPipeline failed; unable to resolve bundles." << Endl;
			return false;
		}

		// Get connected permutation.
		pipelineBuilder->getProfiler()->begin(L"MeshPipeline getConnectedPermutation");
		materialShaderGraph = render::ShaderGraphStatic(materialShaderGraph, materialShaderGraphId).getConnectedPermutation();
		pipelineBuilder->getProfiler()->end();
		if (!materialShaderGraph)
		{
			log::error << L"MeshPipeline failed; unable to freeze connected conditionals, material shader \"" << materialName << L"\"." << Endl;
			return false;
		}

		// Extract platform permutation.
		pipelineBuilder->getProfiler()->begin(L"MeshPipeline getPlatformPermutation");
		materialShaderGraph = render::ShaderGraphStatic(materialShaderGraph, materialShaderGraphId).getPlatformPermutation(m_platform);
		pipelineBuilder->getProfiler()->end();
		if (!materialShaderGraph)
		{
			log::error << L"MeshPipeline failed; unable to get platform \"" << m_platform << L"\" permutation." << Endl;
			return false;
		}

		// Extract renderer permutation.
		pipelineBuilder->getProfiler()->begin(L"MeshPipeline getRendererSignature");
		const wchar_t* rendererSignature = programCompiler->getRendererSignature();
		materialShaderGraph = render::ShaderGraphStatic(materialShaderGraph, materialShaderGraphId).getRendererPermutation(rendererSignature);
		pipelineBuilder->getProfiler()->end();
		if (!materialShaderGraph)
		{
			log::error << L"MeshPipeline failed; unable to get renderer permutation." << Endl;
			return false;
		}

		// Freeze types, get typed permutation.
		pipelineBuilder->getProfiler()->begin(L"MeshPipeline getTypePermutation");
		materialShaderGraph = render::ShaderGraphStatic(materialShaderGraph, materialShaderGraphId).getTypePermutation();
		pipelineBuilder->getProfiler()->end();
		if (!materialShaderGraph)
		{
			log::error << L"MeshPipeline failed; unable to freeze types, material shader \"" << materialName << L"\"." << Endl;
			return false;
		}

		// Cleanup unused branches.
		pipelineBuilder->getProfiler()->begin(L"MeshPipeline removeUnusedBranches");
		materialShaderGraph = render::ShaderGraphOptimizer(materialShaderGraph).removeUnusedBranches(true);
		pipelineBuilder->getProfiler()->end();
		if (!materialShaderGraph)
		{
			log::error << L"MeshPipeline failed; unable to cleanup shader, material shader \"" << materialName << L"\"." << Endl;
			return false;
		}

		// Update bone count from model.
		for (auto node : materialShaderGraph->getNodes())
		{
			if (render::IndexedUniform* indexedUniform = dynamic_type_cast< render::IndexedUniform* >(node))
			{
				if (
					indexedUniform->getParameterName() == L"Mesh_Joints" ||
					indexedUniform->getParameterName() == L"Mesh_LastJoints")
				{
					// Quantize joint count to reduce number of vertex shader permutations as it
					// will cost more than excessive parameters.
					const int32_t uniformJointCount = alignUp(jointCount, 4);
					if (uniformJointCount * 2 != indexedUniform->getLength())
						indexedUniform->setLength(uniformJointCount * 2); // Each bone is represented of a quaternion and a vector thus multiply by 2.
				}
			}
		}

		// Extract each material technique.
		render::ShaderGraphTechniques techniques(materialShaderGraph, materialShaderGraphId);
		if (!techniques.valid())
		{
			log::error << L"MeshPipeline failed; unable to generate techniques, material \"" << materialName << L"\"." << Endl;
			return false;
		}

		std::set< std::wstring > materialTechniqueNames = techniques.getNames();
		if (!m_includeOnlyTechniques.empty())
		{
			std::set< std::wstring > keepTechniqueNames;
			for (const auto& includeOnlyTechniques : m_includeOnlyTechniques)
			{
				WildCompare wc(includeOnlyTechniques);
				for (const auto& materialTechniqueName : materialTechniqueNames)
					if (wc.match(materialTechniqueName))
						keepTechniqueNames.insert(materialTechniqueName);
			}
			materialTechniqueNames = keepTechniqueNames;
		}

		// Generate graph for each technique.
		log::info << L"Mesh material \"" << materialName << L"\" techniques:" << Endl;
		for (const auto& materialTechniqueName : materialTechniqueNames)
		{
			Ref< render::ShaderGraph > materialTechniqueShaderGraph = DeepClone(techniques.generate(materialTechniqueName)).create< render::ShaderGraph >();

			const uint32_t hash = render::ShaderGraphHash(true, false).calculate(materialTechniqueShaderGraph);
			const std::wstring shaderTechniqueName = str(L"M/%s/%08x", materialTechniqueName.c_str(), hash);

			for (auto node : materialTechniqueShaderGraph->getNodes())
			{
				if (auto vertexOutputNode = dynamic_type_cast< render::VertexOutput* >(node))
					vertexOutputNode->setTechnique(shaderTechniqueName);
				if (auto pixelOutputNode = dynamic_type_cast< render::PixelOutput* >(node))
					pixelOutputNode->setTechnique(shaderTechniqueName);
			}

			materialTechniqueShaderGraphs[hash] = materialTechniqueShaderGraph;

			MeshMaterialTechnique mt;
			mt.worldTechnique = materialTechniqueName;
			mt.shaderTechnique = shaderTechniqueName;
			mt.hash = hash;
			materialTechniqueMap[materialName].push_back(mt);

			log::info << L"\t\"" << materialTechniqueName << L"\"\t\t(" << shaderTechniqueName << L")" << Endl;
		}

		// Build vertex declaration from shader vertex inputs.
		for (auto vertexInputNode : materialShaderGraph->findNodesOf< render::VertexInput >())
		{
			render::DataType elementDataType = vertexInputNode->getDataType();
			if (m_promoteHalf)
			{
				if (elementDataType == render::DtHalf2)
					elementDataType = render::DtFloat2;
				else if (elementDataType == render::DtHalf4)
					elementDataType = render::DtFloat4;
			}

			// Is it already added to vertex declaration?
			bool elementDeclared = false;
			for (const auto& vertexElement : vertexElements)
			{
				if (
					vertexInputNode->getDataUsage() == vertexElement.getDataUsage() &&
					vertexInputNode->getIndex() == vertexElement.getIndex())
				{
					if (elementDataType != vertexElement.getDataType())
						log::warning << L"Identical vertex input usage but different types (" << render::getDataTypeName(elementDataType) << L" and " << render::getDataTypeName(vertexElement.getDataType()) << L")" << Endl;
					elementDeclared = true;
					break;
				}
			}
			if (!elementDeclared)
			{
				render::VertexElement element(
					vertexInputNode->getDataUsage(),
					elementDataType,
					0,
					vertexInputNode->getIndex());
				vertexElements.push_back(element);
			}
		}
	}

	// Sort vertex declaration and calculate offsets.
	std::stable_sort(vertexElements.begin(), vertexElements.end(), [](const render::VertexElement& lh, const render::VertexElement& rh) {
		return lh.getSize() > rh.getSize();
	});
	std::stable_sort(vertexElements.begin(), vertexElements.end(), [](const render::VertexElement& lh, const render::VertexElement& rh) {
		return lh.getIndex() < rh.getIndex();
	});
	std::stable_sort(vertexElements.begin(), vertexElements.end(), [](const render::VertexElement& lh, const render::VertexElement& rh) {
		return lh.getDataUsage() < rh.getDataUsage();
	});
	for (auto& vertexElement : vertexElements)
	{
		vertexElement = render::VertexElement(
			vertexElement.getDataUsage(),
			vertexElement.getDataType(),
			vertexElementOffset,
			vertexElement.getIndex());
		vertexElementOffset += vertexElement.getSize();
	}
	log::info << L"Mesh using " << vertexElements.size() << L" vertex elements." << Endl;

	// Merge all shader technique fragments into a single material shader.
	Ref< render::ShaderGraph > materialShaderGraph = new render::ShaderGraph();
	for (auto i = materialTechniqueShaderGraphs.begin(); i != materialTechniqueShaderGraphs.end(); ++i)
	{
		Ref< render::ShaderGraph > materialTechniqueShaderGraph = i->second;
		for (auto node : materialTechniqueShaderGraph->getNodes())
			materialShaderGraph->addNode(node);
		for (auto edge : materialTechniqueShaderGraph->getEdges())
			materialShaderGraph->addEdge(edge);
	}

	// Build material shader.
	const std::wstring materialPath = Path(outputPath).getPathOnly() + L"/" + outputGuid.format() + L"/Shader";
	if (!pipelineBuilder->buildAdHocOutput(
			materialShaderGraph,
			materialPath,
			materialGuid))
	{
		log::error << L"Mesh pipeline failed; unable to build material shader." << Endl;
		return false;
	}

	// Create render mesh.
	Ref< MeshResource > resource = converter->createResource();
	if (!resource)
	{
		log::error << L"Mesh pipeline failed; unable to create mesh resource." << Endl;
		return false;
	}

	log::info << L"Creating mesh resource \"" << type_name(resource) << L"\"..." << Endl;

	// Create output instance.
	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid);
	if (!outputInstance)
	{
		log::error << L"Mesh pipeline failed; unable to create output instance." << Endl;
		return false;
	}

	// Open asset data stream.
	Ref< IStream > stream = outputInstance->writeData(L"Data");
	if (!stream)
	{
		log::error << L"Mesh pipeline failed; unable to create mesh data stream." << Endl;
		outputInstance->revert();
		return false;
	}

	if (/* compressed */ true)
	{
		stream = new compress::DeflateStreamLzf(stream);
		resource->setCompressed(true);
	}

	// Convert mesh asset.
	if (!converter->convert(
			asset,
			model,
			materialGuid,
			materialTechniqueMap,
			vertexElements,
			resource,
			stream))
	{
		log::error << L"Mesh pipeline failed; unable to convert mesh." << Endl;
		return false;
	}

	stream->close();

	// Commit resource.
	outputInstance->setObject(resource);
	if (!outputInstance->commit())
	{
		log::error << L"Mesh pipeline failed; unable to commit output instance." << Endl;
		return false;
	}

	return true;
}

Ref< ISerializable > MeshPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams) const
{
	T_FATAL_ERROR;
	return nullptr;
}

render::IProgramCompiler* MeshPipeline::getProgramCompiler() const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_programCompilerLock);

	if (m_programCompiler)
		return m_programCompiler;

	const TypeInfo* programCompilerType = TypeInfo::find(m_programCompilerTypeName.c_str());
	if (!programCompilerType)
	{
		log::error << L"Mesh pipeline; unable to find program compiler type \"" << m_programCompilerTypeName << L"\"." << Endl;
		return nullptr;
	}

	m_programCompiler = dynamic_type_cast< render::IProgramCompiler* >(programCompilerType->createInstance());
	if (!m_programCompiler)
	{
		log::error << L"Mesh pipeline; unable to instanciate program compiler \"" << m_programCompilerTypeName << L"\"." << Endl;
		return nullptr;
	}

	return m_programCompiler;
}

}
