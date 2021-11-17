#include "Core/Io/IStream.h"
#include "Core/Misc/SafeDestroy.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
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

// bool TerrainEntityReplicator::addDependencies(
//     editor::IPipelineDepends* pipelineDepends,
//     const world::EntityData* entityData,
//     const world::IEntityComponentData* componentData
// ) const
// {
// 	const TerrainComponentData* terrainComponentData = mandatory_non_null_type_cast< const TerrainComponentData* >(componentData);
//     pipelineDepends->addDependency(terrainComponentData->getTerrain(), editor::PdfUse);
// 	return true;
// }

Ref< model::Model > TerrainEntityReplicator::createVisualModel(
    editor::IPipelineBuilder* pipelineBuilder,
	const world::EntityData* entityData,
	const world::IEntityComponentData* componentData
) const
{
	const TerrainComponentData* terrainComponentData = mandatory_non_null_type_cast< const TerrainComponentData* >(componentData);
    const resource::Id< terrain::Terrain >& terrain = terrainComponentData->getTerrain();

    Ref< const terrain::TerrainAsset > terrainAsset = pipelineBuilder->getObjectReadOnly< terrain::TerrainAsset >(terrain);
    if (!terrain)
        return nullptr;

    Ref< db::Instance > heightfieldAssetInstance = pipelineBuilder->getSourceDatabase()->getInstance(terrainAsset->getHeightfield());
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
    editor::IPipelineBuilder* pipelineBuilder,
	const world::EntityData* entityData,
	const world::IEntityComponentData* componentData
) const
{
    return nullptr;
}

// Ref< Object > TerrainEntityReplicator::modifyOutput(
//     editor::IPipelineBuilder* /*pipelineBuilder*/,
//     const world::EntityData* /*entityData*/,
//     const world::IEntityComponentData* componentData,
//     const model::Model* /*model*/,
//     const Guid& /*outputGuid*/
// ) const
// {
//     return const_cast< world::IEntityComponentData* >(componentData);
// }

    }
}