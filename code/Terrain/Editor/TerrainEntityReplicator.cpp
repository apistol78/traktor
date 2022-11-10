/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/IStream.h"
#include "Core/Misc/SafeDestroy.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IPipelineCommon.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldFormat.h"
#include "Heightfield/Editor/ConvertHeightfield.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Terrain/TerrainComponentData.h"
#include "Terrain/Editor/TerrainAsset.h"
#include "Terrain/Editor/TerrainEntityReplicator.h"
#include "World/IEntityComponentData.h"

namespace traktor
{
    namespace terrain
    {

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.TerrainEntityReplicator", 0, TerrainEntityReplicator, scene::IEntityReplicator)

bool TerrainEntityReplicator::create(const editor::IPipelineSettings* settings)
{
    return true;
}

TypeInfoSet TerrainEntityReplicator::getSupportedTypes() const
{
    return makeTypeInfoSet< TerrainComponentData >();
}

Ref< model::Model > TerrainEntityReplicator::createVisualModel(
    editor::IPipelineCommon* pipelineCommon,
	const world::EntityData* entityData,
	const world::IEntityComponentData* componentData
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

    return hf::ConvertHeightfield().convert(heightfield, 64, heightfieldAsset->getVistaDistance());
}

Ref< model::Model > TerrainEntityReplicator::createCollisionModel(
    editor::IPipelineCommon* pipelineCommon,
	const world::EntityData* entityData,
	const world::IEntityComponentData* componentData
) const
{
    return nullptr;
}

void TerrainEntityReplicator::transform(
    world::EntityData* entityData,
    world::IEntityComponentData* componentData,
    world::GroupComponentData* outputGroup
) const
{
}

    }
}