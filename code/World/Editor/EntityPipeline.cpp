#include "World/Editor/EntityPipeline.h"
#include "World/Entity/EntityInstance.h"
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

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.world.EntityPipeline", EntityPipeline, editor::IPipeline)

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
	typeSet.insert(&type_of< EntityInstance >());
	typeSet.insert(&type_of< EntityData >());
	return typeSet;
}

bool EntityPipeline::buildDependencies(
	editor::PipelineManager* pipelineManager,
	const db::Instance* sourceInstance,
	const Serializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	if (const EntityInstance* instance = dynamic_type_cast< const EntityInstance* >(sourceAsset))
		pipelineManager->addDependency(instance->getEntityData());
	else if (const ExternalEntityData* externalEntityData = dynamic_type_cast< const ExternalEntityData* >(sourceAsset))
		pipelineManager->addDependency(externalEntityData->getGuid());
	else if (const ExternalSpatialEntityData* externalSpatialEntityData = dynamic_type_cast< const ExternalSpatialEntityData* >(sourceAsset))
		pipelineManager->addDependency(externalSpatialEntityData->getGuid());
	else if (const GroupEntityData* groupEntityData = dynamic_type_cast< const GroupEntityData* >(sourceAsset))
	{
		const RefArray< EntityInstance >& instances = groupEntityData->getInstances();
		for (RefArray< EntityInstance >::const_iterator i = instances.begin(); i != instances.end(); ++i)
			pipelineManager->addDependency(*i);
	}
	else if (const SpatialGroupEntityData* spatialGroupEntityData = dynamic_type_cast< const SpatialGroupEntityData* >(sourceAsset))
	{
		const RefArray< EntityInstance >& instances = spatialGroupEntityData->getInstances();
		for (RefArray< EntityInstance >::const_iterator i = instances.begin(); i != instances.end(); ++i)
			pipelineManager->addDependency(*i);
	}
	return true;
}

bool EntityPipeline::buildOutput(
	editor::PipelineManager* pipelineManager,
	const Serializable* sourceAsset,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	if ((reason & (BrSourceModified | BrForced)) == 0)
		return true;

	Ref< db::Instance > outputInstance = pipelineManager->createOutputInstance(outputPath, outputGuid);
	if (!outputInstance)
		return false;

	outputInstance->setObject(sourceAsset);

	if (!outputInstance->commit())
		return false;

	return true;
}

	}
}
