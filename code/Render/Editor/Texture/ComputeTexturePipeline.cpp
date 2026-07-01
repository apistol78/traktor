/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Texture/ComputeTexturePipeline.h"

#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Render/Compute/ComputeTextureResource.h"
#include "Render/Editor/Texture/ComputeTextureAsset.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ComputeTexturePipeline", 0, ComputeTexturePipeline, editor::DefaultPipeline)

TypeInfoSet ComputeTexturePipeline::getAssetTypes() const
{
	return makeTypeInfoSet< ComputeTextureAsset >();
}

bool ComputeTexturePipeline::shouldCache() const
{
	return true;
}

bool ComputeTexturePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
    const ComputeTextureAsset* asset = mandatory_non_null_type_cast< const ComputeTextureAsset* >(sourceAsset);
    pipelineDepends->addDependency(asset->getShader(), editor::PdfBuild);
	return true;
}

bool ComputeTexturePipeline::buildOutput(
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
	const ComputeTextureAsset* asset = mandatory_non_null_type_cast< const ComputeTextureAsset* >(sourceAsset);

	Ref< ComputeTextureResource > resource = new ComputeTextureResource();
    resource->m_shader = asset->getShader();
    resource->m_width = asset->getWidth();
    resource->m_height = asset->getHeight();
    resource->m_format = asset->getFormat();
    resource->m_continuous = asset->isContinuous();

	return pipelineBuilder->buildAdHocOutput(
		resource,
		outputPath,
		outputGuid
	);
}

}
