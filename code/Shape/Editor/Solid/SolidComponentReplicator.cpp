/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Database/Database.h"
#include "Editor/IPipelineCommon.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/Operations/Boolean.h"
#include "Model/Operations/CleanDegenerate.h"
#include "Model/Operations/MergeCoplanarAdjacents.h"
#include "Model/Operations/Transform.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/StaticBodyDesc.h"
#include "Physics/Editor/MeshAsset.h"
#include "Physics/World/RigidBodyComponentData.h"
#include "Shape/Editor/Solid/IShape.h"
#include "Shape/Editor/Solid/PrimitiveComponentData.h"
#include "Shape/Editor/Solid/SolidComponentData.h"
#include "Shape/Editor/Solid/SolidComponentReplicator.h"
#include "World/EntityData.h"
#include "World/Entity/GroupComponentData.h"

namespace traktor::shape
{
//		namespace
//		{
//
//void associateMaterials(
//	db::Database* database,
//	model::Model* model,
//	const SmallMap< int32_t, Guid >& materialMap
//)
//{
//	uint32_t tc = model->addUniqueTexCoordChannel(L"UVMap");
//
//	AlignedVector< model::Material > materials = model->getMaterials();
//	for (const auto& m : materialMap)
//	{
//		if (m.first >= materials.size())
//			continue;
//
//		Ref< Material > sm = database->getObjectReadOnly< Material >(m.second);
//		if (!sm)
//			continue;
//
//		auto& material = materials[m.first];
//		sm->prepareMaterial(tc, material);
//	}
//	model->setMaterials(materials);	
//}
//
//		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.SolidComponentReplicator", 0, SolidComponentReplicator, world::IEntityReplicator)

bool SolidComponentReplicator::create(const editor::IPipelineSettings* settings)
{
    return true;
}

TypeInfoSet SolidComponentReplicator::getSupportedTypes() const
{
	return makeTypeInfoSet< SolidComponentData >();
}

RefArray< const world::IEntityComponentData > SolidComponentReplicator::getDependentComponents(
	const world::EntityData* entityData,
	const world::IEntityComponentData* componentData
) const
{
	RefArray< const world::IEntityComponentData > dependentComponentData;
	dependentComponentData.push_back(componentData);
	return dependentComponentData;
}

Ref< model::Model > SolidComponentReplicator::createModel(
	editor::IPipelineCommon* pipelineCommon,
    const world::EntityData* entityData,
    const world::IEntityComponentData* componentData,
	Usage usage
) const
{
	const SolidComponentData* solidComponentData = mandatory_non_null_type_cast< const SolidComponentData* >(componentData);
	
	auto group = entityData->getComponent< world::GroupComponentData >();
	if (!group)
		return nullptr;

	// Get all primitive entities with shape.
	RefArray< const world::EntityData > primitiveEntityDatas;
	for (auto entityData : group->getEntityData())
	{
		if (entityData->getComponent< PrimitiveComponentData >() != nullptr)
			primitiveEntityDatas.push_back(entityData);
	}

	// Merge all primitives.
	model::Model current;

	auto it = primitiveEntityDatas.begin();
	if (it != primitiveEntityDatas.end())
	{
		const world::EntityData* firstEntityData = *it;
		const PrimitiveComponentData* firstPrimitiveComponentData = firstEntityData->getComponent< PrimitiveComponentData >();

		auto model = firstPrimitiveComponentData->getShape()->createModel();
		if (!model)
			return nullptr;

		//associateMaterials(
		//	pipelineBuilder->getSourceDatabase(),
		//	model,
		//	(*it)->getMaterials()
		//);

		current = *model;
		model::Transform(firstEntityData->getTransform().toMatrix44()).apply(current);

		for (++it; it != primitiveEntityDatas.end(); ++it)
		{
			const world::EntityData* entityData = *it;
			const PrimitiveComponentData* primitiveComponentData = entityData->getComponent< PrimitiveComponentData >();

			auto other = primitiveComponentData->getShape()->createModel();
			if (!other)
				continue;

			//associateMaterials(
			//	pipelineBuilder->getSourceDatabase(),
			//	other,
			//	(*it)->getMaterials()
			//);

			model::Model result;

			switch (primitiveComponentData->getOperation())
			{
			case BooleanOperation::Union:
				{
					model::Boolean(
						current,
						Transform::identity(),
						*other,
						entityData->getTransform(),
						model::Boolean::BoUnion
					).apply(result);
				}
				break;

			case BooleanOperation::Intersection:
				{
					model::Boolean(
						current,
						Transform::identity(),
						*other,
						entityData->getTransform(),
						model::Boolean::BoIntersection
					).apply(result);
				}
				break;

			case BooleanOperation::Difference:
				{
					model::Boolean(
						current,
						Transform::identity(),
						*other,
						entityData->getTransform(),
						model::Boolean::BoDifference
					).apply(result);
				}
				break;
			}

			current = std::move(result);
			model::CleanDegenerate().apply(current);
			model::MergeCoplanarAdjacents().apply(current);
		}
	}

	model::Transform(entityData->getTransform().inverse().toMatrix44()).apply(current);

	return new model::Model(current);
}

}
