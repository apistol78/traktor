/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
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
#include "Shape/Editor/Solid/PrimitiveEntityData.h"
#include "Shape/Editor/Solid/SolidEntityData.h"
#include "Shape/Editor/Solid/SolidEntityReplicator.h"
#include "World/EntityData.h"
#include "World/Entity/GroupComponentData.h"

namespace traktor
{
	namespace shape
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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.SolidEntityReplicator", 0, SolidEntityReplicator, scene::IEntityReplicator)

bool SolidEntityReplicator::create(const editor::IPipelineSettings* settings)
{
    return true;
}

TypeInfoSet SolidEntityReplicator::getSupportedTypes() const
{
	return makeTypeInfoSet< SolidEntityData >();
}

Ref< model::Model > SolidEntityReplicator::createVisualModel(
	editor::IPipelineCommon* pipelineCommon,
    const world::EntityData* entityData,
    const world::IEntityComponentData* componentData
) const
{
	const SolidEntityData* solidEntityData = mandatory_non_null_type_cast< const SolidEntityData* >(entityData);
	
	auto group = solidEntityData->getComponent< world::GroupComponentData >();
	if (!group)
		return nullptr;

	// Get all primitive entities with shape.
	RefArray< const PrimitiveEntityData > primitiveEntityDatas;
	for (auto entityData : group->getEntityData())
	{
		if (const auto primitiveEntityData = dynamic_type_cast< const PrimitiveEntityData* >(entityData))
		{
			if (primitiveEntityData->getShape() != nullptr)
				primitiveEntityDatas.push_back(primitiveEntityData);
		}
	}

	// Merge all primitives.
	model::Model current;

	auto it = primitiveEntityDatas.begin();
	if (it != primitiveEntityDatas.end())
	{
		auto model = (*it)->getShape()->createModel();
		if (!model)
			return nullptr;

		//associateMaterials(
		//	pipelineBuilder->getSourceDatabase(),
		//	model,
		//	(*it)->getMaterials()
		//);

		current = *model;
		model::Transform((*it)->getTransform().toMatrix44()).apply(current);

		for (++it; it != primitiveEntityDatas.end(); ++it)
		{
			auto other = (*it)->getShape()->createModel();
			if (!other)
				continue;

			//associateMaterials(
			//	pipelineBuilder->getSourceDatabase(),
			//	other,
			//	(*it)->getMaterials()
			//);

			model::Model result;

			switch ((*it)->getOperation())
			{
			case BooleanOperation::Union:
				{
					model::Boolean(
						current,
						Transform::identity(),
						*other,
						(*it)->getTransform(),
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
						(*it)->getTransform(),
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
						(*it)->getTransform(),
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

	model::Transform(solidEntityData->getTransform().inverse().toMatrix44()).apply(current);

	return new model::Model(current);
}

Ref< model::Model > SolidEntityReplicator::createCollisionModel(
	editor::IPipelineCommon* pipelineCommon,
    const world::EntityData* entityData,
    const world::IEntityComponentData* componentData
) const
{
	return nullptr;
}

	}
}