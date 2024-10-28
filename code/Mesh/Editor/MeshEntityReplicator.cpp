/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Settings/PropertyObject.h"
#include "Core/Settings/PropertyString.h"
#include "Drawing/Image.h"
#include "Editor/IPipelineCommon.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Mesh/Editor/MeshEntityReplicator.h"
#include "Model/Model.h"
#include "Model/ModelCache.h"
#include "Model/Operations/Transform.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderGraphPreview.h"
#include "Render/Editor/Texture/TextureSet.h"
#include "World/EntityData.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.MeshEntityReplicator", 0, MeshEntityReplicator, world::IEntityReplicator)

bool MeshEntityReplicator::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.AssetPath");
	m_modelCachePath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.ModelCache.Path");
	return true;
}

TypeInfoSet MeshEntityReplicator::getSupportedTypes() const
{
    return makeTypeInfoSet< MeshComponentData >();
}

RefArray< const world::IEntityComponentData > MeshEntityReplicator::getDependentComponents(
	const world::EntityData* entityData,
	const world::IEntityComponentData* componentData
) const
{
	RefArray< const world::IEntityComponentData > dependentComponentData;
	dependentComponentData.push_back(componentData);
	return dependentComponentData;
}

Ref< model::Model > MeshEntityReplicator::createModel(
	editor::IPipelineCommon* pipelineCommon,
	const world::EntityData* entityData,
	const world::IEntityComponentData* componentData,
	Usage usage
) const
{
	if (usage != Usage::Visual)
		return nullptr;

	const MeshComponentData* meshComponentData = mandatory_non_null_type_cast< const MeshComponentData* >(componentData);

	// Get referenced mesh asset.
	Ref< const MeshAsset > meshAsset = pipelineCommon->getObjectReadOnly< MeshAsset >(meshComponentData->getMesh());
	if (!meshAsset)
		return nullptr;

	// Read source model.
	Path filePath = FileSystem::getInstance().getAbsolutePath(Path(m_assetPath) + meshAsset->getFileName());
	Ref< model::Model > model = model::ModelCache::getInstance().getMutable(m_modelCachePath, filePath, meshAsset->getImportFilter());
	if (!model)
		return nullptr;

	model->apply(model::Transform(
		translate(meshAsset->getOffset()) *
		scale(meshAsset->getScaleFactor(), meshAsset->getScaleFactor(), meshAsset->getScaleFactor())
	));

	// Create a mesh asset; used by bake pipeline to set appropriate materials.
	Ref< mesh::MeshAsset > outputMeshAsset = new mesh::MeshAsset();
	outputMeshAsset->setMeshType(mesh::MeshAsset::MtStatic);
	outputMeshAsset->setMaterialShaders(meshAsset->getMaterialShaders());

	const auto& materialTextures = meshAsset->getMaterialTextures();
	const auto& materialShaders = meshAsset->getMaterialShaders();

	// Bind texture references in material maps.
	for (auto& material : model->getMaterials())
	{
		auto diffuseMap = material.getDiffuseMap();
		auto it = materialTextures.find(diffuseMap.name);
		if (it != materialTextures.end())
		{
			diffuseMap.texture = it->second;
			material.setDiffuseMap(diffuseMap);
		}
	}

	for (auto& material : model->getMaterials())
	{
		const auto it = materialShaders.find(material.getName());
		if (it != materialShaders.end())
		{
			const Ref< const render::ShaderGraph > materialShaderGraph = pipelineCommon->getObjectReadOnly< render::ShaderGraph >(it->second);
			if (!materialShaderGraph)
				continue;
		
			Ref< drawing::Image > image = render::ShaderGraphPreview(m_assetPath, pipelineCommon->getSourceDatabase()).generate(materialShaderGraph, 128, 128);
			if (!image)
				continue;

			auto diffuseMap = material.getDiffuseMap();
			diffuseMap.image = image;			
			material.setDiffuseMap(diffuseMap);
		}
	}

	outputMeshAsset->setMaterialTextures(materialTextures);

	model->setProperty< PropertyObject >(type_name< MeshAsset >(), outputMeshAsset);
	return model;
}

}
