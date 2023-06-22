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
	Ref< model::Model > model = model::ModelCache(m_modelCachePath).get(filePath, meshAsset->getImportFilter());
	if (!model)
		return nullptr;

	model::Transform(
		translate(meshAsset->getOffset()) *
		scale(meshAsset->getScaleFactor(), meshAsset->getScaleFactor(), meshAsset->getScaleFactor())
	).apply(*model);

	// Create a mesh asset; used by bake pipeline to set appropriate materials.
	Ref< mesh::MeshAsset > outputMeshAsset = new mesh::MeshAsset();
	outputMeshAsset->setMeshType(mesh::MeshAsset::MtStatic);
	outputMeshAsset->setMaterialTemplates(meshAsset->getMaterialTemplates());
	outputMeshAsset->setMaterialShaders(meshAsset->getMaterialShaders());

	// Create list of texture references.
	std::map< std::wstring, Guid > materialTextures;

	// First use textures from texture set.
	const auto& textureSetId = meshAsset->getTextureSet();
	if (textureSetId.isNotNull())
	{
		Ref< const render::TextureSet > textureSet = pipelineCommon->getObjectReadOnly< render::TextureSet >(textureSetId);
		if (!textureSet)
			return nullptr;

		materialTextures = textureSet->get();
	}

	// Then let explicit material textures override those from a texture set.
	for (const auto& mt : meshAsset->getMaterialTextures())
		materialTextures[mt.first] = mt.second;

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

	outputMeshAsset->setMaterialTextures(materialTextures);

	model->setProperty< PropertyObject >(type_name< MeshAsset >(), outputMeshAsset);
	return model;
}

void MeshEntityReplicator::transform(
	world::EntityData* entityData,
	world::IEntityComponentData* componentData,
	world::GroupComponentData* outputGroup
) const
{
	entityData->removeComponent(componentData);
}

}
