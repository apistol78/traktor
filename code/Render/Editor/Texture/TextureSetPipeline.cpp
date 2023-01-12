/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Editor/IPipelineDepends.h"
#include "Render/Editor/Texture/TextureSet.h"
#include "Render/Editor/Texture/TextureSetPipeline.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TextureSetPipeline", 0, TextureSetPipeline, editor::DefaultPipeline)

TypeInfoSet TextureSetPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< TextureSet >();
}

bool TextureSetPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const TextureSet* textureSet = checked_type_cast< const TextureSet* >(sourceAsset);
	for (const auto& ts : textureSet->get())
		pipelineDepends->addDependency(ts.second, editor::PdfBuild);
	return true;
}

bool TextureSetPipeline::buildOutput(
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
	return true;
}

Ref< ISerializable > TextureSetPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	T_FATAL_ERROR;
	return nullptr;
}

}
