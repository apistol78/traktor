#include "Terrain/Editor/TerrainEntityPipeline.h"
#include "Terrain/TerrainEntityData.h"
#include "Terrain/TerrainSurface.h"
#include "Terrain/OceanEntityData.h"
#include "Terrain/UndergrowthEntityData.h"
#include "Editor/PipelineManager.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.terrain.TerrainEntityPipeline", TerrainEntityPipeline, world::EntityPipeline)

TypeSet TerrainEntityPipeline::getAssetTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< TerrainEntityData >());
	typeSet.insert(&type_of< OceanEntityData >());
	typeSet.insert(&type_of< UndergrowthEntityData >());
	return typeSet;
}

bool TerrainEntityPipeline::buildDependencies(
	editor::PipelineManager* pipelineManager,
	const Object* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	if (const TerrainEntityData* terrainEntityData = dynamic_type_cast< const TerrainEntityData* >(sourceAsset))
	{
		pipelineManager->addDependency(terrainEntityData->getHeightfield().getGuid());
		pipelineManager->addDependency(terrainEntityData->getShader().getGuid());

		const Ref< TerrainSurface >& surface = terrainEntityData->getSurface();
		if (surface)
		{
			const std::vector< resource::Proxy< render::Shader > >& layers = surface->getLayers();
			for (std::vector< resource::Proxy< render::Shader > >::const_iterator i = layers.begin(); i != layers.end(); ++i)
				pipelineManager->addDependency(i->getGuid());
		}
	}
	else if (const OceanEntityData* oceanEntityData = dynamic_type_cast< const OceanEntityData* >(sourceAsset))
	{
		pipelineManager->addDependency(oceanEntityData->getHeightfield().getGuid());
		pipelineManager->addDependency(oceanEntityData->getShader().getGuid());
	}
	else if (const UndergrowthEntityData* undergrowthEntityData = dynamic_type_cast< const UndergrowthEntityData* >(sourceAsset))
	{
		pipelineManager->addDependency(undergrowthEntityData->getHeightfield().getGuid());
		pipelineManager->addDependency(undergrowthEntityData->getMaterialMask().getGuid());
		pipelineManager->addDependency(undergrowthEntityData->getShader().getGuid());
	}
	return true;
}

	}
}
