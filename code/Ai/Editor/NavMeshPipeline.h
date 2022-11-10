/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Editor/DefaultPipeline.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AI_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace scene
	{

class IEntityReplicator;

	}

	namespace ai
	{

/*! Navigation mesh pipeline.
 * \ingroup AI
 */
class T_DLLCLASS NavMeshPipeline : public editor::DefaultPipeline
{
	T_RTTI_CLASS;

public:
	NavMeshPipeline();

	virtual bool create(const editor::IPipelineSettings* settings) override final;

	virtual TypeInfoSet getAssetTypes() const override final;

	virtual bool shouldCache() const override final;

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

private:
	std::wstring m_assetPath;
	bool m_editor;
	bool m_build;
	int32_t m_terrainStepSize;
	SmallMap< const TypeInfo*, Ref< const scene::IEntityReplicator > > m_entityReplicators;
};

	}
}

