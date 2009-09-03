#include "Physics/Editor/PhysicsPipeline.h"
#include "Physics/BodyDesc.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/HeightfieldShapeDesc.h"
#include "Editor/IPipelineManager.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.physics.PhysicsPipeline", PhysicsPipeline, editor::IPipeline)

bool PhysicsPipeline::create(const editor::Settings* settings)
{
	return true;
}

void PhysicsPipeline::destroy()
{
}

uint32_t PhysicsPipeline::getVersion() const
{
	return 1;
}

TypeSet PhysicsPipeline::getAssetTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< BodyDesc >());
	return typeSet;
}

bool PhysicsPipeline::buildDependencies(
	editor::IPipelineManager* pipelineManager,
	const db::Instance* sourceInstance,
	const Serializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	if (const BodyDesc* bodyDesc = dynamic_type_cast< const BodyDesc* >(sourceAsset))
	{
		if (const MeshShapeDesc* meshShapeDesc = dynamic_type_cast< const MeshShapeDesc* >(bodyDesc->getShape()))
			pipelineManager->addDependency(meshShapeDesc->getMesh().getGuid(), true);
		else if (const HeightfieldShapeDesc* heightfieldShapeDesc = dynamic_type_cast< const HeightfieldShapeDesc* >(bodyDesc->getShape()))
			pipelineManager->addDependency(heightfieldShapeDesc->getHeightfield().getGuid(), true);
	}
	return true;
}

bool PhysicsPipeline::buildOutput(
	editor::IPipelineManager* pipelineManager,
	const Serializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	return true;
}

	}
}
