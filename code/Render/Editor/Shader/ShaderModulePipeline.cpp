/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Misc/Preprocessor.h"
#include "Core/Serialization/DeepHash.h"
#include "Database/Instance.h"
#include "Editor/IPipelineDepends.h"
#include "Render/Editor/Shader/ShaderModule.h"
#include "Render/Editor/Shader/ShaderModulePipeline.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ShaderModulePipeline", 0, ShaderModulePipeline, editor::IPipeline)

bool ShaderModulePipeline::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void ShaderModulePipeline::destroy()
{
}

TypeInfoSet ShaderModulePipeline::getAssetTypes() const
{
	return makeTypeInfoSet< ShaderModule >();
}

bool ShaderModulePipeline::shouldCache() const
{
	return false;
}

uint32_t ShaderModulePipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
}

bool ShaderModulePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const ShaderModule* shaderModule = mandatory_non_null_type_cast< const ShaderModule* >(sourceAsset);

	const std::wstring unprocessedText = shaderModule->escape([&](const Guid& g) -> std::wstring {
		return g.format();
	});

	Preprocessor preprocessor;

	std::wstring text;
	std::set< std::wstring > usings;
	if (!preprocessor.evaluate(unprocessedText, text, usings))
	{
		log::error << L"Shader module pipeline failed; unable to preprocess module " << sourceInstance->getGuid().format() << L"." << Endl;
		return false;
	}

	for (const auto& u : usings)
		pipelineDepends->addDependency(Guid(u), editor::PdfUse);

	return true;
}

bool ShaderModulePipeline::buildOutput(
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
	return false;
}

Ref< ISerializable > ShaderModulePipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	return nullptr;
}

}
