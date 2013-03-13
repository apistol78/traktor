#include "Animation/Editor/AnimatedMeshEntityPipeline.h"
#include "Animation/AnimatedMeshEntityData.h"
#include "Animation/IPoseControllerData.h"
#include "Editor/IPipelineDepends.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.AnimatedMeshEntityPipeline", 0, AnimatedMeshEntityPipeline, world::EntityPipeline)

TypeInfoSet AnimatedMeshEntityPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< AnimatedMeshEntityData >());
	return typeSet;
}

bool AnimatedMeshEntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (const AnimatedMeshEntityData* meshEntityData = dynamic_type_cast< const AnimatedMeshEntityData* >(sourceAsset))
	{
		pipelineDepends->addDependency(meshEntityData->getMesh(), editor::PdfBuild);
		pipelineDepends->addDependency(meshEntityData->getSkeleton(), editor::PdfBuild);
		pipelineDepends->addDependency(meshEntityData->getPoseControllerData());
	}
	return true;
}

	}
}
