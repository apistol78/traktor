/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Physics/BodyDesc.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/HeightfieldShapeDesc.h"
#include "Physics/Editor/PhysicsPipeline.h"
#include "Physics/World/RigidBodyComponentData.h"
#include "Physics/World/Character/CharacterComponentData.h"
#include "Physics/World/Vehicle/VehicleComponentData.h"
#include "Editor/IPipelineDepends.h"
#include "World/IEntityEventData.h"

namespace traktor::physics
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.PhysicsPipeline", 2, PhysicsPipeline, editor::IPipeline)

bool PhysicsPipeline::create(const editor::IPipelineSettings* settings, db::Database* database)
{
	return true;
}

void PhysicsPipeline::destroy()
{
}

TypeInfoSet PhysicsPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< BodyDesc >();
	typeSet.insert< CharacterComponentData >();
	typeSet.insert< RigidBodyComponentData >();
	typeSet.insert< VehicleComponentData >();
	return typeSet;
}

bool PhysicsPipeline::shouldCache() const
{
	return false;
}

uint32_t PhysicsPipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
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
	else if (auto characterData = dynamic_type_cast< const CharacterComponentData* >(sourceAsset))
	{
		for (auto id : characterData->getCollisionGroup())
			pipelineDepends->addDependency(id, editor::PdfBuild | editor::PdfResource);
		for (auto id : characterData->getCollisionMask())
			pipelineDepends->addDependency(id, editor::PdfBuild | editor::PdfResource);
		for (auto id : characterData->getTraceInclude())
			pipelineDepends->addDependency(id, editor::PdfBuild | editor::PdfResource);
		for (auto id : characterData->getTraceIgnore())
			pipelineDepends->addDependency(id, editor::PdfBuild | editor::PdfResource);
	}
	else if (auto rigidBodyData = dynamic_type_cast< const RigidBodyComponentData* >(sourceAsset))
	{
		pipelineDepends->addDependency(rigidBodyData->getBodyDesc());
		pipelineDepends->addDependency(rigidBodyData->getEventCollide());
	}
	else if (auto vehicleData = dynamic_type_cast< const VehicleComponentData* >(sourceAsset))
	{
		for (auto id : vehicleData->getTraceInclude())
			pipelineDepends->addDependency(id, editor::PdfBuild | editor::PdfResource);
		for (auto id : vehicleData->getTraceIgnore())
			pipelineDepends->addDependency(id, editor::PdfBuild | editor::PdfResource);
	}
	return true;
}

bool PhysicsPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::PipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	return true;
}

Ref< ISerializable > PhysicsPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	return DeepClone(sourceAsset).create();
}

}
