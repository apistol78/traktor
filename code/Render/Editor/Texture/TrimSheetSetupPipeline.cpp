/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Texture/TrimSheetSetupPipeline.h"

#include "Render/Editor/Texture/TrimSheetSetupAsset.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TrimSheetSetupPipeline", 0, TrimSheetSetupPipeline, editor::DefaultPipeline)

TypeInfoSet TrimSheetSetupPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< TrimSheetSetupAsset >();
}

bool TrimSheetSetupPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	// Source images are dependencies of each texture, only those of the texture's layer.
	return true;
}

bool TrimSheetSetupPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::PipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	// Nothing to output; textures are built by TrimSheetTexturePipeline.
	return true;
}

}
