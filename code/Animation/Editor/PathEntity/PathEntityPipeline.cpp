#include "Animation/Editor/PathEntity/PathEntityPipeline.h"
#include "Animation/PathEntity/PathEntityData.h"
#include "World/Entity/EntityInstance.h"
#include "Editor/IPipelineDepends.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.PathEntityPipeline", 0, PathEntityPipeline, world::EntityPipeline)

TypeInfoSet PathEntityPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< PathEntityData >());
	return typeSet;
}

bool PathEntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	if (const PathEntityData* pathEntityData = dynamic_type_cast< const PathEntityData* >(sourceAsset))
		pipelineDepends->addDependency(pathEntityData->getInstance());

	return world::EntityPipeline::buildDependencies(pipelineDepends, sourceInstance, sourceAsset, outBuildParams);
}

	}
}
