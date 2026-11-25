/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Terrain/Editor/AlignToTerrainOperator.h"

#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Random.h"
#include "Core/Math/Transform.h"
#include "Core/Math/Vector4.h"
#include "Core/Misc/SafeDestroy.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IPipelineCommon.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/Editor/HeightfieldAssetLayer.h"
#include "Heightfield/Heightfield.h"
#include "Model/Model.h"
#include "Scene/Editor/SceneAsset.h"
#include "Terrain/Editor/AlignToTerrainOperationData.h"
#include "Terrain/Editor/TerrainAsset.h"
#include "Terrain/Editor/TerrainEditData.h"
#include "Terrain/TerrainComponent.h"
#include "Terrain/TerrainComponentData.h"
#include "World/Editor/IEntityReplicator.h"
#include "World/Editor/Traverser.h"
#include "World/Entity.h"
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/GroupComponentData.h"
#include "World/EntityData.h"
#include "World/World.h"

namespace traktor::terrain
{
namespace
{

/*! Maximum number of terrain samples taken along each footprint axis. */
constexpr int32_t c_maxFootprintSamples = 8;

/*! Maximum number of samples taken across an entire footprint. */
constexpr int32_t c_maxFootprintSamplesTotal = c_maxFootprintSamples * c_maxFootprintSamples;

/*! Guard against dividing by a near horizontal entity up axis. */
constexpr float c_minUpY = 0.001f;

/*! Maximum entity nesting depth traversed when measuring bounding boxes. */
constexpr int32_t c_maxEntityDepth = 8;

/*! Pipeline common interface backed by a scene operator transform context.
 * Lets entity replicators generate models both at pipeline time and from editor preview.
 */
class TransformContextPipelineCommon : public editor::IPipelineCommon
{
public:
	explicit TransformContextPipelineCommon(const scene::ISceneOperator::TransformContext& context)
		: m_context(context)
	{
	}

	virtual db::Database* getSourceDatabase() const override final
	{
		return m_context.getSourceDatabase();
	}

	virtual Ref< const ISerializable > getObjectReadOnly(const Guid& instanceGuid) override final
	{
		return m_context.getObjectReadOnly(instanceGuid);
	}

private:
	const scene::ISceneOperator::TransformContext& m_context;
};

/*! Project rotated bounding box onto XZ plane; out min and max are relative to entity origin. */
void projectFootprint(const Aabb3& boundingBox, const Quaternion& rotation, Vector4& outMin, Vector4& outMax)
{
	Vector4 extents[8];
	boundingBox.getExtents(extents);

	outMin = outMax = rotation * extents[0].xyz0();
	for (int32_t i = 1; i < (int32_t)sizeof_array(extents); ++i)
	{
		const Vector4 corner = rotation * extents[i].xyz0();
		outMin = min(outMin, corner);
		outMax = max(outMax, corner);
	}
}

/*! Number of samples required to cover a footprint axis at heightfield resolution. */
int32_t footprintSampleCount(float extent, float cellSize)
{
	if (extent <= cellSize)
		return 1;
	return clamp((int32_t)(extent / cellSize) + 2, 2, c_maxFootprintSamples);
}

//! Terrain height, and optionally normal, from loaded tile of instantiated terrain or else from terrain data.
float sampleTerrain(const TerrainComponent* terrainComponent, TerrainEditData* terrainData, float worldX, float worldZ, Vector4* outNormal)
{
	const Vector4& worldExtent = terrainData->getWorldExtent();
	const int32_t tileCount = terrainData->getTileCount();
	const int32_t tileX = clamp((int32_t)((worldX / worldExtent.x() + 0.5f) * tileCount), 0, tileCount - 1);
	const int32_t tileZ = clamp((int32_t)((worldZ / worldExtent.z() + 0.5f) * tileCount), 0, tileCount - 1);

	const hf::Heightfield* heightfield = nullptr;
	if (terrainComponent && terrainComponent->getTileCount() == tileCount * tileCount)
		heightfield = terrainComponent->getTileHeightfield(tileX + tileZ * tileCount);

	float gridX, gridZ;
	if (heightfield)
	{
		if (outNormal)
		{
			heightfield->worldToGrid(worldX, worldZ, gridX, gridZ);
			*outNormal = heightfield->normalAt(gridX, gridZ);
		}
		return heightfield->getWorldHeight(worldX, worldZ);
	}

	if (outNormal)
	{
		terrainData->worldToGrid(worldX, worldZ, gridX, gridZ);
		*outNormal = terrainData->normalAt(gridX, gridZ, true);
	}
	return terrainData->getWorldHeight(worldX, worldZ, true);
}

/*! Sample terrain below entity's projected footprint, relative to entity origin, at no coarser resolution than terrain.
 * Footprint smaller than a cell collapse into a single sample at entity origin; out normals are optional, return number of samples.
 */
int32_t sampleFootprint(
	const TerrainComponent* terrainComponent,
	TerrainEditData* terrainData,
	float worldX,
	float worldZ,
	const Vector4& footprintMin,
	const Vector4& footprintMax,
	Vector4* outPositions,
	Vector4* outNormals)
{
	const float cellSize = terrainData->getWorldExtent().x() / (float)terrainData->getSize();
	const float extentX = footprintMax.x() - footprintMin.x();
	const float extentZ = footprintMax.z() - footprintMin.z();

	const int32_t countX = footprintSampleCount(extentX, cellSize);
	const int32_t countZ = footprintSampleCount(extentZ, cellSize);

	int32_t count = 0;
	for (int32_t iz = 0; iz < countZ; ++iz)
	{
		const float fz = (countZ > 1) ? (float)iz / (float)(countZ - 1) : 0.5f;
		const float worldSampleZ = worldZ + footprintMin.z() + extentZ * fz;

		for (int32_t ix = 0; ix < countX; ++ix)
		{
			const float fx = (countX > 1) ? (float)ix / (float)(countX - 1) : 0.5f;
			const float worldSampleX = worldX + footprintMin.x() + extentX * fx;

			outPositions[count] = Vector4(
				worldSampleX,
				sampleTerrain(terrainComponent, terrainData, worldSampleX, worldSampleZ, outNormals ? &outNormals[count] : nullptr),
				worldSampleZ,
				1.0f);

			++count;
		}
	}

	return count;
}

/*! Resolve entity world Y from terrain samples, each sample yield Y which place entity's local XZ plane through it.
 * Ground fit picks which of those candidates is used.
 */
float resolveHeight(
	AlignToTerrainOperationData::GroundFit groundFit,
	const Vector4& up,
	float worldX,
	float worldZ,
	const Vector4* positions,
	int32_t count)
{
	const float upX = up.x();
	const float upY = up.y();
	const float upZ = up.z();

	// An entity tipped onto its side has no well defined base plane; fall back
	// to plain terrain heights.
	const bool degenerate = (upY < c_minUpY);

	float lowest = 0.0f;
	float highest = 0.0f;
	float total = 0.0f;

	for (int32_t i = 0; i < count; ++i)
	{
		const Vector4& p = positions[i];
		const float worldY = degenerate
			? p.y()
			: (upX * (p.x() - worldX) + upY * p.y() + upZ * (p.z() - worldZ)) / upY;

		if (i == 0)
			lowest = highest = worldY;
		else
		{
			lowest = min(lowest, worldY);
			highest = max(highest, worldY);
		}

		total += worldY;
	}

	switch (groundFit)
	{
	case AlignToTerrainOperationData::GroundFit::Highest:
		return highest;

	case AlignToTerrainOperationData::GroundFit::Average:
		return total / (float)count;

	default:
		return lowest;
	}
}

const world::EntityData* findTerrainInScene(scene::SceneAsset* sceneAsset)
{
	const world::EntityData* terrainEntityData = nullptr;
	for (auto layer : sceneAsset->getLayers())
	{
		if (!layer)
			continue;

		world::Traverser::visit(layer, [&](const world::EntityData* entityData) -> world::Traverser::Result {
			if (!terrainEntityData && entityData->getComponent< TerrainComponentData >() != nullptr)
				terrainEntityData = entityData;
			return world::Traverser::Result::Continue;
		});

		if (terrainEntityData)
			break;
	}
	return terrainEntityData;
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.AlignToTerrainOperator", 0, AlignToTerrainOperator, scene::ISceneOperator)

bool AlignToTerrainOperator::create(const editor::IPipelineSettings* settings)
{
	// Create entity replicators; used to measure entity footprints.
	for (const auto& entityReplicatorType : type_of< world::IEntityReplicator >().findAllOf(false))
	{
		Ref< world::IEntityReplicator > entityReplicator = mandatory_non_null_type_cast< world::IEntityReplicator* >(entityReplicatorType->createInstance());
		if (!entityReplicator->create(settings))
			return false;

		for (auto supportedType : entityReplicator->getSupportedTypes())
			m_entityReplicators[supportedType] = entityReplicator;
	}

	return true;
}

void AlignToTerrainOperator::destroy()
{
}

TypeInfoSet AlignToTerrainOperator::getOperatorTypes() const
{
	return makeTypeInfoSet< AlignToTerrainOperationData >();
}

void AlignToTerrainOperator::addDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const ISerializable* operatorData) const
{
}

bool AlignToTerrainOperator::isGeometricTransform() const
{
	return true;
}

void AlignToTerrainOperator::gatherGeometry(
	editor::IPipelineCommon* pipelineCommon,
	const world::EntityData* entityData,
	const Transform& transform,
	float contactRatio,
	AlignedVector< GeometryPlacement >& outPlacements,
	boundingBoxCache_t& inoutCache,
	int32_t depth) const
{
	if (!entityData || depth >= c_maxEntityDepth)
		return;

	// External entities are measured once and reused for every placement; only contact box is known, not models.
	if (auto externalEntityData = dynamic_type_cast< const world::ExternalEntityData* >(entityData))
	{
		const Guid& externalId = externalEntityData->getEntityData();

		Aabb3 externalBoundingBox;
		const auto it = inoutCache.find(externalId);
		if (it != inoutCache.end())
			externalBoundingBox = it->second;
		else
		{
			const Ref< const world::EntityData > referencedEntityData = pipelineCommon->getObjectReadOnly< world::EntityData >(externalId);
			externalBoundingBox = calculateContactBoundingBox(pipelineCommon, referencedEntityData, contactRatio, inoutCache, depth + 1);
			inoutCache[externalId] = externalBoundingBox;
		}

		if (!externalBoundingBox.empty())
			outPlacements.push_back({ nullptr, transform, externalBoundingBox.transform(transform) });
	}

	// Geometry contributed by the entity's own components.
	for (auto componentData : entityData->getComponents())
	{
		if (!componentData || is_a< TerrainComponentData >(componentData))
			continue;

		const auto it = m_entityReplicators.find(&type_of(componentData));
		if (it == m_entityReplicators.end())
			continue;

		const Ref< const model::Model > model = it->second->createModel(
			pipelineCommon,
			entityData,
			componentData,
			world::IEntityReplicator::Usage::Visual,
			world::IEntityReplicator::Flags::SkipMaterials);
		if (!model)
			continue;

		const Aabb3 modelBoundingBox = model->getBoundingBox();
		if (!modelBoundingBox.empty())
			outPlacements.push_back({ model, transform, modelBoundingBox.transform(transform) });
	}

	// Geometry contributed by child entities.
	if (auto groupComponentData = entityData->getComponent< world::GroupComponentData >())
		for (auto childEntityData : groupComponentData->getEntityData())
			gatherGeometry(
				pipelineCommon,
				childEntityData,
				transform * childEntityData->getTransform(),
				contactRatio,
				outPlacements,
				inoutCache,
				depth + 1);
}

Aabb3 AlignToTerrainOperator::calculateContactBoundingBox(
	editor::IPipelineCommon* pipelineCommon,
	const world::EntityData* entityData,
	float contactRatio,
	boundingBoxCache_t& inoutCache,
	int32_t depth) const
{
	AlignedVector< GeometryPlacement > placements;
	gatherGeometry(pipelineCommon, entityData, Transform::identity(), contactRatio, placements, inoutCache, depth);

	// Full extent of the entity; establishes where its ground contact slab ends.
	Aabb3 boundingBox;
	for (const auto& placement : placements)
		boundingBox.contain(placement.boundingBox);

	if (boundingBox.empty() || contactRatio >= 1.0f)
		return boundingBox;

	const float threshold = boundingBox.mn.y() + (boundingBox.mx.y() - boundingBox.mn.y()) * contactRatio;

	Aabb3 contactBoundingBox;
	for (const auto& placement : placements)
	{
		// Entirely above the slab; contributes nothing.
		if (placement.boundingBox.mn.y() > threshold)
			continue;

		// Entirely within the slab, or no model to refine against.
		if (!placement.model || placement.boundingBox.mx.y() <= threshold)
		{
			contactBoundingBox.contain(placement.boundingBox);
			continue;
		}

		// Straddles the slab; only the geometry below the threshold counts.
		for (const auto& position : placement.model->getPositions())
		{
			const Vector4 p = placement.transform * position;
			if (p.y() <= threshold)
				contactBoundingBox.contain(p);
		}
	}

	// A model can be built entirely from positions above its own bounds, e.g.
	// when it is degenerate; fall back to the full extent rather than nothing.
	return contactBoundingBox.empty() ? boundingBox : contactBoundingBox;
}

bool AlignToTerrainOperator::transform(
	const scene::ISceneOperator::TransformContext& context,
	const ISerializable* operatorData,
	scene::SceneAsset* inoutSceneAsset) const
{
	const AlignToTerrainOperationData* data = mandatory_non_null_type_cast< const AlignToTerrainOperationData* >(operatorData);

	const world::EntityData* terrainEntityData = findTerrainInScene(inoutSceneAsset);
	if (!terrainEntityData)
	{
		log::warning << L"AlignToTerrain; no terrain found in scene, nothing aligned." << Endl;
		return true;
	}

	const Guid terrainId = terrainEntityData->getComponent< TerrainComponentData >()->getTerrain();

	Ref< const TerrainAsset > terrainAsset = context.getObjectReadOnly< TerrainAsset >(terrainId);
	if (!terrainAsset)
		return true;

	Ref< db::Instance > heightfieldInstance = context.getSourceDatabase()->getInstance(terrainAsset->getHeightfield());
	if (!heightfieldInstance)
		return true;

	Ref< const hf::HeightfieldAsset > heightfieldAsset = heightfieldInstance->getObject< const hf::HeightfieldAsset >();
	if (!heightfieldAsset)
		return true;

	// Tiles of terrain data are read when first sampled; data is never edited, thus base layer is given as edit layer.
	Ref< TerrainEditData > terrainData = new TerrainEditData();
	if (!terrainData->create(heightfieldInstance, heightfieldAsset, heightfieldAsset->getLayers().front()->getId()))
		return true;

	// Loaded tiles of instantiated terrain are used if available, avoid reading them from database.
	const TerrainComponent* terrainComponent = nullptr;
	if (world::World* world = context.getWorld())
	{
		world::Entity* terrainEntity = world->getEntity(terrainEntityData->getId());
		terrainComponent = terrainEntity ? terrainEntity->getComponent< TerrainComponent >() : nullptr;
	}

	const AlignedVector< std::wstring >& layerFilters = data->getLayers();
	const AlignToTerrainOperationData::GroundFit groundFit = data->getGroundFit();

	TransformContextPipelineCommon pipelineCommon(context);
	boundingBoxCache_t boundingBoxCache;

	Vector4 samplePositions[c_maxFootprintSamplesTotal];
	Vector4 sampleNormals[c_maxFootprintSamplesTotal];

	uint32_t aligned = 0;
	for (auto layer : inoutSceneAsset->getLayers())
	{
		if (!layer)
			continue;

		if (std::find(layerFilters.begin(), layerFilters.end(), layer->getName()) == layerFilters.end())
			continue;

		auto group = layer->getComponent< world::GroupComponentData >();
		if (!group)
			continue;

		Random rndm;

		for (auto entityData : group->getEntityData())
		{
			if (!entityData)
				continue;

			if (entityData->getComponent< TerrainComponentData >() != nullptr)
				continue;

			const Transform current = entityData->getTransform();
			const Vector4 position = current.translation();
			const float worldX = position.x();
			const float worldZ = position.z();

			// Measure part of entity resting on ground; without measurable geometry, or when fit at center, it collapse to a point.
			Aabb3 boundingBox;
			if (groundFit != AlignToTerrainOperationData::GroundFit::Center)
				boundingBox = calculateContactBoundingBox(&pipelineCommon, entityData, data->getContactRatio(), boundingBoxCache, 0);

			Vector4 footprintMin = Vector4::zero();
			Vector4 footprintMax = Vector4::zero();

			Quaternion rotation = current.rotation();

			if (data->getAlignOrientation())
			{
				// Average surface normal across footprint so large entities follow general slope.
				if (!boundingBox.empty())
					projectFootprint(boundingBox, rotation, footprintMin, footprintMax);

				const int32_t count = sampleFootprint(
					terrainComponent,
					terrainData,
					worldX,
					worldZ,
					footprintMin,
					footprintMax,
					samplePositions,
					sampleNormals);

				Vector4 normal = Vector4::zero();
				for (int32_t i = 0; i < count; ++i)
					normal += sampleNormals[i];

				normal = (normal.length() > FUZZY_EPSILON) ? normal.normalized() : Vector4(0.0f, 1.0f, 0.0f, 0.0f);

				rotation = slerp(
					Quaternion(Vector4(0.0f, 1.0f, 0.0f, 0.0f), normal),
					Quaternion::identity(),
					data->getUpness());
			}

			if (data->getRandomHeadingAngle())
			{
				const float rnd = rndm.nextFloat() * TWO_PI;
				rotation = rotation * Quaternion::fromEulerAngles(rnd, 0.0f, 0.0f);
			}

			// Footprint depends on the final orientation, hence projected again.
			if (!boundingBox.empty())
				projectFootprint(boundingBox, rotation, footprintMin, footprintMax);

			const int32_t count = sampleFootprint(
				terrainComponent,
				terrainData,
				worldX,
				worldZ,
				footprintMin,
				footprintMax,
				samplePositions,
				nullptr);

			const float worldY = resolveHeight(
									 groundFit,
									 rotation * Vector4(0.0f, 1.0f, 0.0f, 0.0f),
									 worldX,
									 worldZ,
									 samplePositions,
									 count) +
				data->getOffset();

			entityData->setTransform(Transform(
				Vector4(worldX, worldY, worldZ, 1.0f),
				rotation));
			++aligned;
		}
	}

	log::debug << L"AlignToTerrain; aligned " << aligned << L" entities to terrain." << Endl;
	return true;
}

bool AlignToTerrainOperator::build(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* operatorData,
	const db::Instance* sourceInstance,
	scene::SceneAsset* inoutSceneAsset,
	bool rebuild) const
{
	// All alignment happens in transform(); nothing to build here.
	return true;
}

}
