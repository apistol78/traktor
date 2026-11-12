/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Guid.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Transform.h"
#include "Core/Ref.h"
#include "Scene/Editor/ISceneOperator.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::editor
{

class IPipelineCommon;

}

namespace traktor::model
{

class Model;

}

namespace traktor::world
{

class EntityData;
class IEntityReplicator;

}

namespace traktor::terrain
{

/*! Scene operator which vertically aligns entities to the terrain surface.
 *
 * A geometric transform: it snaps the position (and, optionally, the
 * orientation) of the targeted entities onto the terrain heightfield. Because
 * all work happens in transform(), the result is observed consistently by the
 * runtime scene, the navigation mesh and the editor preview.
 *
 * Terrain is sampled across the entity's bounding box projected onto the XZ
 * plane, not only at the entity origin, so that entities which are large
 * compared to the terrain slope are not left partially floating.
 *
 * \ingroup Terrain
 */
class T_DLLCLASS AlignToTerrainOperator : public scene::ISceneOperator
{
	T_RTTI_CLASS;

public:
	virtual bool create(const editor::IPipelineSettings* settings) override final;

	virtual void destroy() override final;

	virtual TypeInfoSet getOperatorTypes() const override final;

	virtual void addDependencies(
		editor::IPipelineDepends* pipelineDepends,
		const ISerializable* operatorData
	) const override final;

	virtual bool isGeometricTransform() const override final;

	virtual bool transform(
		const scene::ISceneOperator::TransformContext& context,
		const ISerializable* operatorData,
		scene::SceneAsset* inoutSceneAsset
	) const override final;

	virtual bool build(
		editor::IPipelineBuilder* pipelineBuilder,
		const ISerializable* operatorData,
		const db::Instance* sourceInstance,
		scene::SceneAsset* inoutSceneAsset,
		bool rebuild
	) const override final;

private:
	/*! A single piece of geometry belonging to an entity. */
	struct GeometryPlacement
	{
		Ref< const model::Model > model;	//!< Source model; null when only bounds are known.
		Transform transform;				//!< Model into entity local space.
		Aabb3 boundingBox;					//!< Model bounds in entity local space.
	};

	/*! Contact bounding boxes of already resolved external entities, keyed by their instance id. */
	typedef SmallMap< Guid, Aabb3 > boundingBoxCache_t;

	SmallMap< const TypeInfo*, Ref< const world::IEntityReplicator > > m_entityReplicators;

	/*! Collect geometry, through entity replicators, of an entity in it's own local space.
	 * External entities are resolved into their cached contact bounding box instead of their models.
	 */
	void gatherGeometry(
		editor::IPipelineCommon* pipelineCommon,
		const world::EntityData* entityData,
		const Transform& transform,
		float contactRatio,
		AlignedVector< GeometryPlacement >& outPlacements,
		boundingBoxCache_t& inoutCache,
		int32_t depth
	) const;

	/*! Ground contact bounding box of entity in it's own local space, empty if entity has no measurable geometry.
	 * Only geometry within a slab, contact ratio of height from lowest point, is measured; trunk of a tree rather than canopy.
	 */
	Aabb3 calculateContactBoundingBox(
		editor::IPipelineCommon* pipelineCommon,
		const world::EntityData* entityData,
		float contactRatio,
		boundingBoxCache_t& inoutCache,
		int32_t depth
	) const;
};

}
