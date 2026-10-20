/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Scene/Editor/EntityStreamingOperator.h"

#include "Core/Log/Log.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Editor/IPipelineBuilder.h"
#include "Model/Model.h"
#include "Scene/Editor/EntityStreamingOperationData.h"
#include "Scene/Editor/SceneAsset.h"
#include "World/Editor/IEntityReplicator.h"
#include "World/Entity/EntityStreamingComponentData.h"
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/GroupComponentData.h"
#include "World/Entity/PropertyEntityReference.h"
#include "World/EntityData.h"

#include <cmath>

namespace traktor::scene
{
namespace
{

/*! Collect id of every entity referenced through an entity property. */
void collectReferencedEntities(const ISerializable* object, SmallSet< Guid >& outIds, SmallSet< const ISerializable* >& visited)
{
	if (object == nullptr || !visited.insert(object))
		return;

	if (auto reference = dynamic_type_cast< const world::PropertyEntityReference* >(object))
	{
		if (reference->getEntityId().isNotNull())
			outIds.insert(reference->getEntityId());
		return;
	}

	Ref< Reflection > reflection = Reflection::create(object);
	if (!reflection)
		return;

	RefArray< ReflectionMember > objectMembers;
	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);
	for (const ReflectionMember* member : objectMembers)
		collectReferencedEntities(static_cast< const RfmObject* >(member)->get(), outIds, visited);
}

/*! Resolve entity data referenced by external entity; non external entity is returned as is. */
Ref< const world::EntityData > resolveEntity(editor::IPipelineCommon* pipelineCommon, const world::EntityData* entityData)
{
	auto externalEntityData = dynamic_type_cast< const world::ExternalEntityData* >(entityData);
	if (!externalEntityData)
		return entityData;

	Ref< const world::EntityData > referencedEntityData = resolveEntity(pipelineCommon, pipelineCommon->getObjectReadOnly< world::EntityData >(externalEntityData->getEntityData()));
	if (!referencedEntityData)
		return nullptr;

	Ref< world::EntityData > resolvedEntityData = DeepClone(referencedEntityData).create< world::EntityData >();
	resolvedEntityData->setTransform(externalEntityData->getTransform());
	for (auto componentData : externalEntityData->getComponents())
		resolvedEntityData->setComponent(componentData);

	return resolvedEntityData;
}

/*! Entity, or any of its children, referenced through an entity property. */
bool isReferenced(editor::IPipelineCommon* pipelineCommon, const world::EntityData* entityData, const SmallSet< Guid >& referencedIds)
{
	if (referencedIds.find(entityData->getId()) != referencedIds.end())
		return true;

	Ref< const world::EntityData > resolvedEntityData = resolveEntity(pipelineCommon, entityData);
	if (!resolvedEntityData)
		return false;

	if (auto groupComponentData = resolvedEntityData->getComponent< world::GroupComponentData >())
	{
		for (auto child : groupComponentData->getEntityData())
		{
			if (child && isReferenced(pipelineCommon, child, referencedIds))
				return true;
		}
	}

	return false;
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.EntityStreamingOperator", 0, EntityStreamingOperator, ISceneOperator)

bool EntityStreamingOperator::create(const editor::IPipelineSettings* settings)
{
	for (const auto entityReplicatorType : type_of< world::IEntityReplicator >().findAllOf(false))
	{
		Ref< world::IEntityReplicator > entityReplicator = mandatory_non_null_type_cast< world::IEntityReplicator* >(entityReplicatorType->createInstance());
		if (!entityReplicator->create(settings))
			return false;

		for (auto supportedType : entityReplicator->getSupportedTypes())
			m_entityReplicators[supportedType] = entityReplicator;
	}
	return true;
}

void EntityStreamingOperator::destroy()
{
	m_entityReplicators.clear();
}

TypeInfoSet EntityStreamingOperator::getOperatorTypes() const
{
	return makeTypeInfoSet< EntityStreamingOperationData >();
}

void EntityStreamingOperator::addDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const ISerializable* operatorData) const
{
}

bool EntityStreamingOperator::transform(
	const TransformContext& context,
	const ISerializable* operatorData,
	SceneAsset* inoutSceneAsset) const
{
	// Harvested entities stay in layers for dependencies; they're moved in build().
	return true;
}

bool EntityStreamingOperator::build(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* operatorData,
	const db::Instance* sourceInstance,
	SceneAsset* inoutSceneAsset,
	bool rebuild) const
{
	const auto data = mandatory_non_null_type_cast< const EntityStreamingOperationData* >(operatorData);

	// Referenced entities must always be present in world.
	SmallSet< Guid > referencedIds;
	SmallSet< const ISerializable* > visited;
	collectReferencedEntities(inoutSceneAsset, referencedIds, visited);

	SmallMap< uint32_t, Aabb3 > modelBoundingBoxes;
	RefArray< world::EntityData > entityData;
	AlignedVector< Aabb3 > boundingBoxes;
	for (auto layer : inoutSceneAsset->getLayers())
	{
		if (!layer || !layer->getState().visible || layer->getState().dynamic)
			continue;
		if (auto groupComponentData = layer->getComponent< world::GroupComponentData >())
			harvestEntities(pipelineBuilder, groupComponentData, referencedIds, modelBoundingBoxes, entityData, boundingBoxes);
	}
	if (entityData.empty())
		return true;

	// Group entities into grid cells by center of their bounding box.
	const float cellSize = data->getCellSize();
	SmallMap< std::pair< int32_t, int32_t >, world::EntityStreamingComponentData::Cell > cells;
	for (size_t i = 0; i < entityData.size(); ++i)
	{
		const Vector4 center = boundingBoxes[i].getCenter();
		const int32_t cx = (int32_t)std::floor((float)center.x() / cellSize);
		const int32_t cz = (int32_t)std::floor((float)center.z() / cellSize);

		auto& cell = cells[std::make_pair(cx, cz)];
		cell.boundingBox.contain(boundingBoxes[i]);
		cell.entityData.push_back(entityData[i]);
	}

	AlignedVector< world::EntityStreamingComponentData::Cell > outputCells;
	for (const auto& it : cells)
		outputCells.push_back(it.second);

	Ref< world::EntityStreamingComponentData > entityStreamingComponentData = new world::EntityStreamingComponentData();
	entityStreamingComponentData->setLoadDistance(data->getLoadDistance());
	entityStreamingComponentData->setUnloadDistance(std::max(data->getUnloadDistance(), data->getLoadDistance()));
	entityStreamingComponentData->setCells(outputCells);
	inoutSceneAsset->setWorldComponent(entityStreamingComponentData);

	log::info << L"Entity streaming; " << (int32_t)entityData.size() << L" entities in " << (int32_t)outputCells.size() << L" cells." << Endl;
	return true;
}

void EntityStreamingOperator::harvestEntities(
	editor::IPipelineCommon* pipelineCommon,
	world::GroupComponentData* groupComponentData,
	const SmallSet< Guid >& referencedIds,
	SmallMap< uint32_t, Aabb3 >& inoutModelBoundingBoxes,
	RefArray< world::EntityData >& outEntityData,
	AlignedVector< Aabb3 >& outBoundingBoxes) const
{
	const RefArray< world::EntityData > children = groupComponentData->getEntityData();
	for (auto child : children)
	{
		if (!child || !child->getState().visible || child->getState().dynamic)
			continue;

		// Groups without components of their own are split so children stream individually.
		auto childGroupComponentData = child->getComponent< world::GroupComponentData >();
		if (childGroupComponentData != nullptr && child->getComponents().size() == 1)
		{
			harvestEntities(pipelineCommon, childGroupComponentData, referencedIds, inoutModelBoundingBoxes, outEntityData, outBoundingBoxes);
			continue;
		}

		// Entities without geometry, such as lights or scripts, cannot be measured and are kept.
		const Aabb3 boundingBox = calculateBoundingBox(pipelineCommon, child, inoutModelBoundingBoxes);
		if (boundingBox.empty() || isReferenced(pipelineCommon, child, referencedIds))
		{
			if (childGroupComponentData)
				harvestEntities(pipelineCommon, childGroupComponentData, referencedIds, inoutModelBoundingBoxes, outEntityData, outBoundingBoxes);
			continue;
		}

		groupComponentData->removeEntityData(child);
		outEntityData.push_back(child);
		outBoundingBoxes.push_back(boundingBox);
	}
}

Aabb3 EntityStreamingOperator::calculateBoundingBox(
	editor::IPipelineCommon* pipelineCommon,
	const world::EntityData* entityData,
	SmallMap< uint32_t, Aabb3 >& inoutModelBoundingBoxes) const
{
	Ref< const world::EntityData > resolvedEntityData = resolveEntity(pipelineCommon, entityData);
	if (!resolvedEntityData)
		return Aabb3();

	Aabb3 boundingBox;
	for (auto componentData : resolvedEntityData->getComponents())
	{
		const auto it = m_entityReplicators.find(&type_of(componentData));
		if (it == m_entityReplicators.end())
			continue;

		const world::IEntityReplicator* entityReplicator = it->second;

		// Model only depends on these components; their hash identifies the model.
		uint32_t hash = DeepHash(componentData).get();
		for (auto dependentComponentData : entityReplicator->getDependentComponents(resolvedEntityData, componentData))
			hash = hash * 31 + DeepHash(dependentComponentData).get();

		Aabb3 modelBoundingBox;
		const auto it2 = inoutModelBoundingBoxes.find(hash);
		if (it2 != inoutModelBoundingBoxes.end())
			modelBoundingBox = it2->second;
		else
		{
			Ref< model::Model > model = entityReplicator->createModel(pipelineCommon, resolvedEntityData, componentData, world::IEntityReplicator::Usage::Visual, world::IEntityReplicator::Flags::SkipMaterials);
			if (!model)
				model = entityReplicator->createModel(pipelineCommon, resolvedEntityData, componentData, world::IEntityReplicator::Usage::Collision, world::IEntityReplicator::Flags::SkipMaterials);
			if (model)
			{
				for (const auto& position : model->getPositions())
					modelBoundingBox.contain(position);
			}
			inoutModelBoundingBoxes.insert(hash, modelBoundingBox);
		}

		if (!modelBoundingBox.empty())
			boundingBox.contain(modelBoundingBox.transform(resolvedEntityData->getTransform()));
	}

	if (auto groupComponentData = resolvedEntityData->getComponent< world::GroupComponentData >())
	{
		for (auto child : groupComponentData->getEntityData())
		{
			if (child)
				boundingBox.contain(calculateBoundingBox(pipelineCommon, child, inoutModelBoundingBoxes));
		}
	}

	return boundingBox;
}

}
