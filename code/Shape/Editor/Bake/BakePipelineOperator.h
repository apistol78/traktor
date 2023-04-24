/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Scene/Editor/IScenePipelineOperator.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class IEntityReplicator;

	}

	namespace shape
	{

class TracerProcessor;

class T_DLLCLASS BakePipelineOperator : public scene::IScenePipelineOperator
{
	T_RTTI_CLASS;

public:
	BakePipelineOperator();

	virtual bool create(const editor::IPipelineSettings* settings) override final;

	virtual void destroy() override final;

	virtual TypeInfoSet getOperatorTypes() const override final;

	virtual void addDependencies(editor::IPipelineDepends* pipelineDepends) const override final;

	virtual bool transform(
		editor::IPipelineCommon* pipelineCommon,
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

	static void setTracerProcessor(TracerProcessor* tracerProcessor);

	static TracerProcessor* getTracerProcessor();

private:
	static Ref< TracerProcessor > ms_tracerProcessor;

	std::wstring m_assetPath;
	std::wstring m_modelCachePath;
	const TypeInfo* m_tracerType;
	std::wstring m_compressionMethod;
	bool m_asynchronous;
	SmallMap< const TypeInfo*, Ref< const world::IEntityReplicator > > m_entityReplicators;
};

	}
}