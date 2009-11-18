#include "Terrain/Editor/TerrainEntityPipeline.h"
#include "Terrain/TerrainEntityData.h"
#include "Terrain/TerrainSurface.h"
#include "Terrain/OceanEntityData.h"
#include "Terrain/UndergrowthEntityData.h"
#include "Editor/IPipelineDepends.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.TerrainEntityPipeline", TerrainEntityPipeline, world::EntityPipeline)

TypeInfoSet TerrainEntityPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< TerrainEntityData >());
	typeSet.insert(&type_of< OceanEntityData >());
	typeSet.insert(&type_of< UndergrowthEntityData >());
	return typeSet;
}

bool TerrainEntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	if (const TerrainEntityData* terrainEntityData = dynamic_type_cast< const TerrainEntityData* >(sourceAsset))
	{
		pipelineDepends->addDependency(terrainEntityData->getHeightfield().getGuid(), true);
		pipelineDepends->addDependency(terrainEntityData->getShader().getGuid(), true);

		const Ref< TerrainSurface >& surface = terrainEntityData->getSurface();
		if (surface)
		{
			const std::vector< resource::Proxy< render::Shader > >& layers = surface->getLayers();
			for (std::vector< resource::Proxy< render::Shader > >::const_iterator i = layers.begin(); i != layers.end(); ++i)
				pipelineDepends->addDependency(i->getGuid(), true);
		}
	}
	else if (const OceanEntityData* oceanEntityData = dynamic_type_cast< const OceanEntityData* >(sourceAsset))
	{
		pipelineDepends->addDependency(oceanEntityData->getHeightfield().getGuid(), true);
		pipelineDepends->addDependency(oceanEntityData->getShader().getGuid(), true);
	}
	else if (const UndergrowthEntityData* undergrowthEntityData = dynamic_type_cast< const UndergrowthEntityData* >(sourceAsset))
	{
		pipelineDepends->addDependency(undergrowthEntityData->getHeightfield().getGuid(), true);
		pipelineDepends->addDependency(undergrowthEntityData->getMaterialMask().getGuid(), true);
		pipelineDepends->addDependency(undergrowthEntityData->getShader().getGuid(), true);
	}
	return true;
}

	}
}
