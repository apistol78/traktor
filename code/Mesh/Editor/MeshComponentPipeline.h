/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Editor/IPipeline.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::mesh
{

class T_DLLCLASS MeshComponentPipeline : public editor::IPipeline
{
	T_RTTI_CLASS;

public:
	virtual bool create(const editor::IPipelineSettings* settings) override final;

	virtual void destroy() override final;

	virtual TypeInfoSet getAssetTypes() const override final;

	virtual bool shouldCache() const override final;

	virtual uint32_t hashAsset(const ISerializable* sourceAsset) const override final;
	
	virtual bool buildDependencies(
		editor::IPipelineDepends* pipelineDepends,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid
	) const override final;

	virtual bool buildOutput(
		editor::IPipelineBuilder* pipelineBuilder,
		const editor::PipelineDependencySet* dependencySet,
		const editor::PipelineDependency* dependency,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		const Object* buildParams,
		uint32_t reason
	) const override final;

	virtual Ref< ISerializable > buildProduct(
		editor::IPipelineBuilder* pipelineBuilder,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const Object* buildParams
	) const override final;
};

}
