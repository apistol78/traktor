/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Scene/Editor/IScenePipelineOperator.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::terrain
{

/*! Scene operator which vertically aligns entities to the terrain surface.
 *
 * A geometric transform: it snaps the position (and, optionally, the
 * orientation) of the targeted entities onto the terrain heightfield. Because
 * all work happens in transform(), the result is observed consistently by the
 * runtime scene, the navigation mesh and the editor preview.
 *
 * \ingroup Terrain
 */
class T_DLLCLASS AlignToTerrainOperator : public scene::IScenePipelineOperator
{
	T_RTTI_CLASS;

public:
	virtual bool create(const editor::IPipelineSettings* settings) override final;

	virtual void destroy() override final;

	virtual TypeInfoSet getOperatorTypes() const override final;

	virtual void addDependencies(editor::IPipelineDepends* pipelineDepends) const override final;

	virtual bool isGeometricTransform() const override final;

	virtual bool transform(
		const scene::IScenePipelineOperator::TransformContext& context,
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
};

}
