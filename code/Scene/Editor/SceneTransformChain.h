/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Scene/Editor/IScenePipelineOperator.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::editor
{

class IPipelineSettings;

}

namespace traktor::scene
{

class IScenePipelineOperator;
class SceneAsset;

/*! Chain of geometric scene operators.
 *
 * Instantiates the geometric scene pipeline operators (those reporting
 * isGeometricTransform()) and applies their transforms to a scene asset. This
 * mirrors the geometric-transform pass the ScenePipeline runs at build time,
 * letting the editor produce the same "scene after PCG" for live preview.
 *
 * Non-geometric operators (e.g. lightmap bake) are intentionally excluded; they
 * are a build-time only concern and must not affect the interactive scene.
 *
 * \ingroup Scene
 */
class T_DLLCLASS SceneTransformChain : public Object
{
	T_RTTI_CLASS;

public:
	bool create(const editor::IPipelineSettings* settings);

	void destroy();

	bool apply(SceneAsset* inoutSceneAsset, const IScenePipelineOperator::TransformContext& context) const;

	bool empty() const { return m_operators.empty(); }

private:
	RefArray< IScenePipelineOperator > m_operators;

	const IScenePipelineOperator* findOperator(const TypeInfo& operationType) const;
};

}
