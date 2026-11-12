/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Scene/Editor/ISceneOperator.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AI_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ai
{

/*! Add chunks of chunked navigation mesh as cells of entity streaming.
 * \ingroup AI
 */
class T_DLLCLASS NavMeshStreamingOperator : public scene::ISceneOperator
{
	T_RTTI_CLASS;

public:
	virtual bool create(const editor::IPipelineSettings* settings) override final;

	virtual void destroy() override final;

	virtual TypeInfoSet getOperatorTypes() const override final;

	virtual void addDependencies(editor::IPipelineDepends* pipelineDepends, const ISerializable* operatorData) const override final;

	virtual bool transform(const scene::ISceneOperator::TransformContext& context, const ISerializable* operatorData, scene::SceneAsset* inoutSceneAsset) const override final;

	virtual bool build(editor::IPipelineBuilder* pipelineBuilder, const ISerializable* operatorData, const db::Instance* sourceInstance, scene::SceneAsset* inoutSceneAsset, bool rebuild) const override final;

private:
	bool m_targetEditor = false;
};

}
