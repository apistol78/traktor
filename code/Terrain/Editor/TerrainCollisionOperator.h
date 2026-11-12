/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Scene/Editor/ISceneOperator.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class EntityData;
class GroupComponentData;

}

namespace traktor::terrain
{

/*! Replace heightfield collision of terrain with one streamed collision entity per terrain tile.
 * \ingroup Terrain
 */
class T_DLLCLASS TerrainCollisionOperator : public scene::ISceneOperator
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
	bool m_targetEditor = false;

	//! Find terrain entities with heightfield collision, recursively, along with their parent group.
	void findTerrainEntities(world::GroupComponentData* groupComponentData, RefArray< world::EntityData >& outEntities, RefArray< world::GroupComponentData >& outParents) const;
};

}
