#include "Physics/Editor/PhysicsPipeline.h"
#include "Physics/BodyDesc.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/HeightfieldShapeDesc.h"
#include "Editor/IPipelineDepends.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.PhysicsPipeline", 1, PhysicsPipeline, editor::IPipeline)

bool PhysicsPipeline::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void PhysicsPipeline::destroy()
{
}

TypeInfoSet PhysicsPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< BodyDesc >());
	return typeSet;
}

bool PhysicsPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	if (const BodyDesc* bodyDesc = dynamic_type_cast< const BodyDesc* >(sourceAsset))
	{
		if (const MeshShapeDesc* meshShapeDesc = dynamic_type_cast< const MeshShapeDesc* >(bodyDesc->getShape()))
			pipelineDepends->addDependency(meshShapeDesc->getMesh().getGuid(), editor::PdfBuild);
		else if (const HeightfieldShapeDesc* heightfieldShapeDesc = dynamic_type_cast< const HeightfieldShapeDesc* >(bodyDesc->getShape()))
			pipelineDepends->addDependency(heightfieldShapeDesc->getHeightfield().getGuid(), editor::PdfBuild);
	}
	return true;
}

bool PhysicsPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset,
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
