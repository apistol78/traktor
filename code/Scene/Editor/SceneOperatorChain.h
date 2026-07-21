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

class IPipelineSettings;

}

namespace traktor::scene
{

class ISceneOperator;
class SceneAsset;

/*! Chain of geometric scene operators.
 *
 * Instantiates the geometric scene operators (those reporting
 * isGeometricTransform()) and applies their transforms to a scene asset. This
 * mirrors the geometric-transform pass the ScenePipeline runs at build time,
 * letting the editor produce the same "scene after operators" for preview.
 *
 * Non-geometric operators (e.g. lightmap bake) are intentionally excluded; they
 * are a build-time only concern and must not affect the interactive scene.
 *
 * \ingroup Scene
 */
class T_DLLCLASS SceneOperatorChain : public Object
{
	T_RTTI_CLASS;

public:
	bool create(const editor::IPipelineSettings* settings);

	void destroy();

	bool apply(SceneAsset* inoutSceneAsset, const ISceneOperator::TransformContext& context) const;

	bool empty() const { return m_operators.empty(); }

private:
	RefArray< ISceneOperator > m_operators;

	const ISceneOperator* findOperator(const TypeInfo& operationType) const;
};

}
