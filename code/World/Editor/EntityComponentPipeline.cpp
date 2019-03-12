#include "Core/Serialization/DeepClone.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "World/IEntityComponentData.h"
#include "World/Editor/EntityComponentPipeline.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.EntityComponentPipeline", 0, EntityComponentPipeline, editor::IPipeline)

bool EntityComponentPipeline::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void EntityComponentPipeline::destroy()
{
}

TypeInfoSet EntityComponentPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< IEntityComponentData >();
}

bool EntityComponentPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	return true;
}

bool EntityComponentPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::IPipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	T_FATAL_ERROR;
	return false;
}

Ref< ISerializable > EntityComponentPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	return DeepClone(sourceAsset).create();
}

	}
}
