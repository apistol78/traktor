#include "World/Editor/EntityPipeline.h"
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/ExternalSpatialEntityData.h"
#include "World/Entity/GroupEntityData.h"
#include "World/Entity/SpatialGroupEntityData.h"
#include "Editor/PipelineManager.h"
#include "Database/Instance.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.world.EntityPipeline", EntityPipeline, editor::Pipeline)

bool EntityPipeline::create(const editor::Settings* settings)
{
	return true;
}

void EntityPipeline::destroy()
{
}

uint32_t EntityPipeline::getVersion() const
{
	return 1;
}

TypeSet EntityPipeline::getAssetTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< EntityData >());
	return typeSet;
}

bool EntityPipeline::buildDependencies(
	editor::PipelineManager* pipelineManager,
	const Object* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	if (const ExternalEntityData* externalEntityData = dynamic_type_cast< const ExternalEntityData* >(sourceAsset))
		pipelineManager->addDependency(externalEntityData->getGuid());

	if (const ExternalSpatialEntityData* externalSpatialEntityData = dynamic_type_cast< const ExternalSpatialEntityData* >(sourceAsset))
		pipelineManager->addDependency(externalSpatialEntityData->getGuid());

	if (const GroupEntityData* groupEntityData = dynamic_type_cast< const GroupEntityData* >(sourceAsset))
	{
		const RefArray< EntityData >& entityData = groupEntityData->getEntityData();
		for (RefArray< EntityData >::const_iterator i = entityData.begin(); i != entityData.end(); ++i)
			pipelineManager->addDependency(*i);
	}

	if (const SpatialGroupEntityData* spatialGroupEntityData = dynamic_type_cast< const SpatialGroupEntityData* >(sourceAsset))
	{
		const RefArray< SpatialEntityData >& entityData = spatialGroupEntityData->getEntityData();
		for (RefArray< SpatialEntityData >::const_iterator i = entityData.begin(); i != entityData.end(); ++i)
			pipelineManager->addDependency(*i);
	}

	return true;
}

bool EntityPipeline::buildOutput(
	editor::PipelineManager* pipelineManager,
	const Object* sourceAsset,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	if ((reason & (BrSourceModified | BrForced)) == 0)
		return true;

	Ref< db::Instance > outputInstance = pipelineManager->createOutputInstance(outputPath, outputGuid, sourceAsset);
	if (!outputInstance)
		return false;

	if (!outputInstance->commit())
		return false;

	return true;
}

	}
}
