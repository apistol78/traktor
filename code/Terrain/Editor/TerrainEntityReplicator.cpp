/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/IStream.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IPipelineCommon.h"
#include "Editor/IPipelineSettings.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldFormat.h"
#include "Heightfield/Editor/ConvertHeightfield.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Model/Model.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderGraphPreview.h"
#include "Terrain/TerrainComponentData.h"
#include "Terrain/Editor/TerrainAsset.h"
#include "Terrain/Editor/TerrainEntityReplicator.h"
#include "World/IEntityComponentData.h"

namespace traktor::terrain
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.TerrainEntityReplicator", 0, TerrainEntityReplicator, world::IEntityReplicator)

bool TerrainEntityReplicator::create(const editor::IPipelineSettings* settings)
{
    m_assetPath = settings->getPropertyIncludeHash< std::wstring >(L"Pipeline.AssetPath", L"");
    return true;
}

TypeInfoSet TerrainEntityReplicator::getSupportedTypes() const
{
    return makeTypeInfoSet< TerrainComponentData >();
}

RefArray< const world::IEntityComponentData > TerrainEntityReplicator::getDependentComponents(
    const world::EntityData* entityData,
    const world::IEntityComponentData* componentData
) const
{
    RefArray< const world::IEntityComponentData > dependentComponentData;
    dependentComponentData.push_back(componentData);
    return dependentComponentData;
}

Ref< model::Model > TerrainEntityReplicator::createModel(
    editor::IPipelineCommon* pipelineCommon,
	const world::EntityData* entityData,
	const world::IEntityComponentData* componentData,
    Usage usage
) const
{
	const TerrainComponentData* terrainComponentData = mandatory_non_null_type_cast< const TerrainComponentData* >(componentData);
    const resource::Id< terrain::Terrain >& terrain = terrainComponentData->getTerrain();

    Ref< const terrain::TerrainAsset > terrainAsset = pipelineCommon->getObjectReadOnly< terrain::TerrainAsset >(terrain);
    if (!terrain)
        return nullptr;

    Ref< db::Instance > heightfieldAssetInstance = pipelineCommon->getSourceDatabase()->getInstance(terrainAsset->getHeightfield());
    if (!heightfieldAssetInstance)
        return nullptr;

    Ref< const hf::HeightfieldAsset > heightfieldAsset = heightfieldAssetInstance->getObject< const hf::HeightfieldAsset >();
    if (!heightfieldAsset)
        return nullptr;

    Ref< IStream > sourceData = heightfieldAssetInstance->readData(L"Data");
    if (!sourceData)
        return nullptr;

    Ref< hf::Heightfield > heightfield = hf::HeightfieldFormat().read(
        sourceData,
        heightfieldAsset->getWorldExtent()
    );
    if (!heightfield)
        return nullptr;

    safeClose(sourceData);

    Ref< model::Model > heightfieldModel = hf::ConvertHeightfield().convert(heightfield, 64);
    if (!heightfieldModel)
        return nullptr;

    Ref< const render::ShaderGraph > surfaceShader = pipelineCommon->getSourceDatabase()->getObjectReadOnly< render::ShaderGraph >(terrainAsset->getSurfaceShader());
    if (surfaceShader)
    {
        Ref< drawing::Image > surfaceImage = render::ShaderGraphPreview(m_assetPath, pipelineCommon->getSourceDatabase()).generate(surfaceShader, 1024, 1024);
        if (surfaceImage)
        {
            AlignedVector< model::Material > materials = heightfieldModel->getMaterials();
            materials.front().setDiffuseMap(model::Material::Map(
                L"Diffuse",
                L"Base",
                true,
                Guid(),
                surfaceImage
            ));
            heightfieldModel->setMaterials(materials);
        }
    }

    return heightfieldModel;
}

}
