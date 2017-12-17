/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/DeepClone.h"
#include "Physics/BodyDesc.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/HeightfieldShapeDesc.h"
#include "Physics/Editor/PhysicsPipeline.h"
#include "Physics/World/RigidBodyComponentData.h"
#include "Physics/World/Vehicle/VehicleComponentData.h"
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
	typeSet.insert(&type_of< VehicleComponentData >());
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
	if (auto bodyDesc = dynamic_type_cast< const BodyDesc* >(sourceAsset))
	{
		const ShapeDesc* shapeDesc = bodyDesc->getShape();
		if (shapeDesc)
		{
			for (auto id : shapeDesc->getCollisionGroup())
				pipelineDepends->addDependency(id, editor::PdfBuild | editor::PdfResource);
			for (auto id : shapeDesc->getCollisionMask())
				pipelineDepends->addDependency(id, editor::PdfBuild | editor::PdfResource);

			if (auto meshShapeDesc = dynamic_type_cast< const MeshShapeDesc* >(shapeDesc))
				pipelineDepends->addDependency(meshShapeDesc->getMesh(), editor::PdfBuild | editor::PdfResource);
			else if (auto heightfieldShapeDesc = dynamic_type_cast< const HeightfieldShapeDesc* >(shapeDesc))
				pipelineDepends->addDependency(heightfieldShapeDesc->getHeightfield(), editor::PdfBuild | editor::PdfResource);
		}
	}
	else if (auto rigidBodyData = dynamic_type_cast< const RigidBodyComponentData* >(sourceAsset))
	{
		pipelineDepends->addDependency(rigidBodyData->getBodyDesc());
		pipelineDepends->addDependency(rigidBodyData->getEventCollide());
	}
	else if (auto vehicleData = dynamic_type_cast< const VehicleComponentData* >(sourceAsset))
	{
		pipelineDepends->addDependency(vehicleData->getBodyDesc());
		for (auto id : vehicleData->getTraceInclude())
			pipelineDepends->addDependency(id, editor::PdfBuild | editor::PdfResource);
		for (auto id : vehicleData->getTraceIgnore())
			pipelineDepends->addDependency(id, editor::PdfBuild | editor::PdfResource);
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
