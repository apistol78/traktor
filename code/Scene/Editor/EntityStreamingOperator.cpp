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
#include "Core/Math/Const.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Model/Model.h"
#include "Render/ITexture.h"
#include "Render/Shader.h"
#include "Scene/Editor/EntityStreamingHLODComponentData.h"
#include "Scene/Editor/EntityStreamingOperationData.h"
#include "Scene/Editor/SceneAsset.h"
#include "World/Editor/BillboardTextureAsset.h"
#include "World/Editor/IEntityReplicator.h"
#include "World/Entity/BillboardComponentData.h"
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

const Guid c_billboardShader(L"{501CF7C5-084D-4294-9F5C-9F94F5017953}");
const Guid c_billboardAlbedoSeed(L"{3A148E23-378A-4561-BFBD-805D3B571CCA}");
const Guid c_billboardNormalSeed(L"{05132967-0D7C-4D29-B2AF-93AB0B9A9EFF}");
const Guid c_billboardEntitySeed(L"{62658050-61A0-42DB-B19A-6B86F40EF839}");

/*! Id of generated billboard texture from hash of its source. */
Guid billboardTextureId(const Guid& seed, uint32_t hash)
{
	uint8_t d[16];
	for (int32_t i = 0; i < 16; ++i)
		d[i] = seed[i];
	for (int32_t i = 0; i < 4; ++i)
		d[i] ^= (uint8_t)(hash >> (i * 8));
	return Guid(d);
}

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
			if (child && isReferenced(pipelineCommon, child, referencedIds))
				return true;
	}

	return false;
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.EntityStreamingOperator", 0, EntityStreamingOperator, ISceneOperator)

bool EntityStreamingOperator::create(const editor::IPipelineSettings* settings)
{
	m_targetEditor = settings->getPropertyIncludeHash< bool >(L"Pipeline.TargetEditor", false);

	if (!m_targetEditor)
	{
		for (const auto entityReplicatorType : type_of< world::IEntityReplicator >().findAllOf(false))
		{
			Ref< world::IEntityReplicator > entityReplicator = mandatory_non_null_type_cast< world::IEntityReplicator* >(entityReplicatorType->createInstance());
			if (!entityReplicator->create(settings))
				return false;

			for (auto supportedType : entityReplicator->getSupportedTypes())
				m_entityReplicators[supportedType] = entityReplicator;
		}
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
	// Shader of generated billboards.
	if (!m_targetEditor)
		pipelineDepends->addDependency(c_billboardShader, editor::PdfBuild | editor::PdfResource);
}

bool EntityStreamingOperator::transform(
	const TransformContext& context,
	const ISerializable* operatorData,
	SceneAsset* inoutSceneAsset) const
{
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

	// Do nothing in case editor is the target; streaming is not supported in editor.
	if (m_targetEditor)
		return true;

	// Referenced entities must always be present in world.
	SmallSet< Guid > referencedIds;
	SmallSet< const ISerializable* > visited;
	collectReferencedEntities(inoutSceneAsset, referencedIds, visited);

	SmallMap< uint32_t, Aabb3 > modelBoundingBoxes;
	RefArray< world::EntityData > entityData;
	AlignedVector< Aabb3 > boundingBoxes;
	for (auto layer : inoutSceneAsset->getLayers())
	{
		if (!layer || !layer->getState().visible || layer->getState().dynamic || layer->getState().resident)
			continue;
		if (auto groupComponentData = layer->getComponent< world::GroupComponentData >())
			harvestEntities(pipelineBuilder, groupComponentData, referencedIds, modelBoundingBoxes, entityData, boundingBoxes);
	}
	if (entityData.empty())
		return true;

	// Group entities into grid cells by center of their bounding box; billboards stand in for entities beyond load distance.
	const float cellSize = data->getCellSize();
	SmallMap< std::pair< int32_t, int32_t >, world::EntityStreamingComponentData::Cell > cells;
	SmallMap< uint32_t, Billboard > billboards;
	int32_t billboardCount = 0;
	for (size_t i = 0; i < entityData.size(); ++i)
	{
		const Vector4 center = boundingBoxes[i].getCenter();
		const int32_t cx = (int32_t)std::floor((float)center.x() / cellSize);
		const int32_t cz = (int32_t)std::floor((float)center.z() / cellSize);

		auto& cell = cells[std::make_pair(cx, cz)];
		cell.boundingBox.contain(boundingBoxes[i]);
		cell.entityData.push_back(entityData[i]);

		Ref< world::EntityData > billboardEntityData = createBillboardEntity(pipelineBuilder, entityData[i], modelBoundingBoxes, billboards);
		if (billboardEntityData)
		{
			cell.hlodEntityData.push_back(billboardEntityData);
			++billboardCount;
		}
	}

	AlignedVector< world::EntityStreamingComponentData::Cell > outputCells;
	for (const auto& it : cells)
		outputCells.push_back(it.second);

	Ref< world::EntityStreamingComponentData > entityStreamingComponentData = new world::EntityStreamingComponentData();
	entityStreamingComponentData->setLoadDistance(data->getLoadDistance());
	entityStreamingComponentData->setUnloadDistance(std::max(data->getUnloadDistance(), data->getLoadDistance()));
	entityStreamingComponentData->setHlodLoadDistance(data->getHlodLoadDistance());
	entityStreamingComponentData->setHlodUnloadDistance(std::max(data->getHlodUnloadDistance(), data->getHlodLoadDistance()));
	entityStreamingComponentData->setCells(outputCells);
	inoutSceneAsset->setWorldComponent(entityStreamingComponentData);

	log::info << L"Entity streaming; " << (int32_t)entityData.size() << L" entities, " << billboardCount << L" billboards, in " << (int32_t)outputCells.size() << L" cells." << Endl;
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
		if (!child || !child->getState().visible || child->getState().dynamic || child->getState().resident)
			continue;

		// Groups without components of their own are split so children stream individually.
		auto childGroupComponentData = child->getComponent< world::GroupComponentData >();
		if (childGroupComponentData != nullptr && child->getComponents().size() == 1)
		{
			harvestEntities(pipelineCommon, childGroupComponentData, referencedIds, inoutModelBoundingBoxes, outEntityData, outBoundingBoxes);
			continue;
		}

		// Entities without geometry, such as lights or scripts, cannot be measured and are kept.
		const Aabb3 boundingBox = calculateBoundingBox(pipelineCommon, child, false, inoutModelBoundingBoxes);
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
	bool visualOnly,
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
		uint32_t hash = DeepHash(componentData).get() + (visualOnly ? 1 : 0);
		for (auto dependentComponentData : entityReplicator->getDependentComponents(resolvedEntityData, componentData))
			hash = hash * 31 + DeepHash(dependentComponentData).get();

		Aabb3 modelBoundingBox;
		const auto it2 = inoutModelBoundingBoxes.find(hash);
		if (it2 != inoutModelBoundingBoxes.end())
			modelBoundingBox = it2->second;
		else
		{
			Ref< model::Model > model = entityReplicator->createModel(pipelineCommon, resolvedEntityData, componentData, world::IEntityReplicator::Usage::Visual, world::IEntityReplicator::Flags::SkipMaterials);
			if (!model && !visualOnly)
				model = entityReplicator->createModel(pipelineCommon, resolvedEntityData, componentData, world::IEntityReplicator::Usage::Collision, world::IEntityReplicator::Flags::SkipMaterials);
			if (model)
				modelBoundingBox = model->getBoundingBox();
			inoutModelBoundingBoxes.insert(hash, modelBoundingBox);
		}

		if (!modelBoundingBox.empty())
			boundingBox.contain(modelBoundingBox.transform(resolvedEntityData->getTransform()));
	}

	if (auto groupComponentData = resolvedEntityData->getComponent< world::GroupComponentData >())
	{
		for (auto child : groupComponentData->getEntityData())
			if (child)
				boundingBox.contain(calculateBoundingBox(pipelineCommon, child, visualOnly, inoutModelBoundingBoxes));
	}

	return boundingBox;
}

Ref< world::EntityData > EntityStreamingOperator::createBillboardEntity(
	editor::IPipelineBuilder* pipelineBuilder,
	const world::EntityData* entityData,
	SmallMap< uint32_t, Aabb3 >& inoutModelBoundingBoxes,
	SmallMap< uint32_t, Billboard >& inoutBillboards) const
{
	Ref< const world::EntityData > resolvedEntityData = resolveEntity(pipelineBuilder, entityData);
	if (!resolvedEntityData)
		return nullptr;

	const EntityStreamingHLODComponentData* hlodData = resolvedEntityData->getComponent< EntityStreamingHLODComponentData >();
	if (!hlodData)
		return nullptr;

	// Billboard is rendered from entity in its own space; entities of same source share billboard.
	Ref< world::EntityData > sourceEntityData = DeepClone(resolvedEntityData).create< world::EntityData >();
	sourceEntityData->setId(Guid());
	sourceEntityData->setName(L"");
	sourceEntityData->setTransform(Transform::identity());

	const uint32_t hash = DeepHash(sourceEntityData).get();

	auto it = inoutBillboards.find(hash);
	if (it == inoutBillboards.end())
	{
		Billboard& billboard = inoutBillboards[hash];

		const Aabb3 boundingBox = calculateBoundingBox(pipelineBuilder, sourceEntityData, true, inoutModelBoundingBoxes);
		if (boundingBox.empty())
		{
			log::warning << L"Entity streaming; unable to create billboard of \"" << entityData->getName() << L"\", no visual geometry." << Endl;
			return nullptr;
		}

		// Square framing which contain entity from every angle; yaw keep the horizontal radius, elevation trade height for depth.
		const Vector4 extent = boundingBox.getExtent();
		const float radiusXZ = (float)(extent * Vector4(1.0f, 0.0f, 1.0f, 0.0f)).length();
		float halfExtent = radiusXZ;
		for (int32_t i = 0; i < hlodData->getElevations(); ++i)
		{
			const float elevation = (hlodData->getElevations() > 1) ? hlodData->getElevationFrom() + (hlodData->getElevationTo() - hlodData->getElevationFrom()) * i / (hlodData->getElevations() - 1) : hlodData->getElevationFrom();
			halfExtent = std::max(halfExtent, (float)extent.y() * std::abs(std::cos(elevation)) + radiusXZ * std::abs(std::sin(elevation)));
		}

		const float size = halfExtent * 2.0f;
		const Vector4 offset = boundingBox.getCenter().xyz1();
		const Guid albedoTexture = billboardTextureId(c_billboardAlbedoSeed, hash);
		const Guid normalTexture = billboardTextureId(c_billboardNormalSeed, hash);

		Ref< world::BillboardTextureAsset > albedoAsset = new world::BillboardTextureAsset(
			sourceEntityData,
			world::BillboardTextureAsset::Content::Albedo,
			hlodData->getAngles(),
			hlodData->getElevations(),
			hlodData->getElevationFrom(),
			hlodData->getElevationTo(),
			hlodData->getTileSize(),
			size,
			offset);

		Ref< world::BillboardTextureAsset > normalAsset = new world::BillboardTextureAsset(
			sourceEntityData,
			world::BillboardTextureAsset::Content::Normals,
			hlodData->getAngles(),
			hlodData->getElevations(),
			hlodData->getElevationFrom(),
			hlodData->getElevationTo(),
			hlodData->getTileSize(),
			size,
			offset);

		if (
			!pipelineBuilder->buildAdHocOutput(albedoAsset, albedoTexture) ||
			!pipelineBuilder->buildAdHocOutput(normalAsset, normalTexture))
		{
			log::error << L"Entity streaming; unable to build billboard textures of \"" << entityData->getName() << L"\"." << Endl;
			return nullptr;
		}

		billboard.albedoTexture = albedoTexture;
		billboard.normalTexture = normalTexture;
		billboard.size = size;
		billboard.offset = offset;

		log::info << L"Entity streaming; billboard of \"" << entityData->getName() << L"\", size " << size << L"." << Endl;
		it = inoutBillboards.find(hash);
	}

	const Billboard& billboard = it->second;
	if (billboard.albedoTexture.isNull())
		return nullptr;

	Ref< world::EntityData > billboardEntityData = new world::EntityData();
	billboardEntityData->setId(entityData->getId().permutation(c_billboardEntitySeed));
	billboardEntityData->setName(entityData->getName());
	billboardEntityData->setTransform(resolvedEntityData->getTransform());
	billboardEntityData->setComponent(new world::BillboardComponentData(
		resource::Id< render::Shader >(c_billboardShader),
		resource::Id< render::ITexture >(billboard.albedoTexture),
		resource::Id< render::ITexture >(billboard.normalTexture),
		hlodData->getAngles(),
		hlodData->getElevations(),
		hlodData->getElevationFrom(),
		hlodData->getElevationTo(),
		billboard.size,
		billboard.offset));
	return billboardEntityData;
}

}
