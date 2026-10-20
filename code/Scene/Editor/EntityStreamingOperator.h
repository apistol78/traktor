/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Containers/SmallSet.h"
#include "Core/Math/Aabb3.h"
#include "Scene/Editor/ISceneOperator.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::editor
{

class IPipelineCommon;

}

namespace traktor::world
{

class EntityData;
class GroupComponentData;
class IEntityReplicator;

}

namespace traktor::scene
{

/*! Move static entities of scene into an entity streaming world component.
 * \ingroup Scene
 */
class T_DLLCLASS EntityStreamingOperator : public ISceneOperator
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

	virtual bool transform(
		const TransformContext& context,
		const ISerializable* operatorData,
		SceneAsset* inoutSceneAsset
	) const override final;

	virtual bool build(
		editor::IPipelineBuilder* pipelineBuilder,
		const ISerializable* operatorData,
		const db::Instance* sourceInstance,
		SceneAsset* inoutSceneAsset,
		bool rebuild
	) const override final;

private:
	SmallMap< const TypeInfo*, Ref< const world::IEntityReplicator > > m_entityReplicators;

	/*! Move static entities out of group and its descendants, along with their bounding boxes. */
	void harvestEntities(
		editor::IPipelineCommon* pipelineCommon,
		world::GroupComponentData* groupComponentData,
		const SmallSet< Guid >& referencedIds,
		SmallMap< uint32_t, Aabb3 >& inoutModelBoundingBoxes,
		RefArray< world::EntityData >& outEntityData,
		AlignedVector< Aabb3 >& outBoundingBoxes
	) const;

	/*! World space bounding box of entity and its children, from models of entity replicators. */
	Aabb3 calculateBoundingBox(
		editor::IPipelineCommon* pipelineCommon,
		const world::EntityData* entityData,
		SmallMap< uint32_t, Aabb3 >& inoutModelBoundingBoxes
	) const;
};

}
