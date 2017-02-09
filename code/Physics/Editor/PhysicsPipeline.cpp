#include "Core/Serialization/DeepClone.h"
#include "Physics/BodyDesc.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/HeightfieldShapeDesc.h"
#include "Physics/Editor/PhysicsPipeline.h"
#include "Physics/World/RigidBodyComponentData.h"
#include "Editor/IPipelineDepends.h"
#include "World/IEntityEventData.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.PhysicsPipeline", 2, PhysicsPipeline, editor::IPipeline)

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
	typeSet.insert(&type_of< RigidBodyComponentData >());
	return typeSet;
}

bool PhysicsPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (const BodyDesc* bodyDesc = dynamic_type_cast< const BodyDesc* >(sourceAsset))
	{
		const ShapeDesc* shapeDesc = bodyDesc->getShape();
		if (shapeDesc)
		{
			for (std::set< resource::Id< CollisionSpecification > >::const_iterator i = shapeDesc->getCollisionGroup().begin(); i != shapeDesc->getCollisionGroup().end(); ++i)
				pipelineDepends->addDependency(*i, editor::PdfBuild | editor::PdfResource);
			for (std::set< resource::Id< CollisionSpecification > >::const_iterator i = shapeDesc->getCollisionMask().begin(); i != shapeDesc->getCollisionMask().end(); ++i)
				pipelineDepends->addDependency(*i, editor::PdfBuild | editor::PdfResource);

			if (const MeshShapeDesc* meshShapeDesc = dynamic_type_cast< const MeshShapeDesc* >(shapeDesc))
				pipelineDepends->addDependency(meshShapeDesc->getMesh(), editor::PdfBuild | editor::PdfResource);
			else if (const HeightfieldShapeDesc* heightfieldShapeDesc = dynamic_type_cast< const HeightfieldShapeDesc* >(shapeDesc))
				pipelineDepends->addDependency(heightfieldShapeDesc->getHeightfield(), editor::PdfBuild | editor::PdfResource);
		}
	}
	else if (const RigidBodyComponentData* componentData = dynamic_type_cast< const RigidBodyComponentData* >(sourceAsset))
	{
		pipelineDepends->addDependency(componentData->getBodyDesc());
		pipelineDepends->addDependency(componentData->getEventCollide());
	}
	return true;
}

bool PhysicsPipeline::buildOutput(
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
	return true;
}

Ref< ISerializable > PhysicsPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	return DeepClone(sourceAsset).create();
}

	}
}
