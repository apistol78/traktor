/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/DeepHash.h"
#include "Editor/IPipelineDepends.h"
#include "UiKit/Editor/Scaffolding.h"
#include "UiKit/Editor/ScaffoldingPipeline.h"

namespace traktor::uikit
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.uikit.ScaffoldingPipeline", 0, ScaffoldingPipeline, editor::IPipeline)

bool ScaffoldingPipeline::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void ScaffoldingPipeline::destroy()
{
}

TypeInfoSet ScaffoldingPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< Scaffolding >();
}

bool ScaffoldingPipeline::shouldCache() const
{
	return false;
}

uint32_t ScaffoldingPipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
}

bool ScaffoldingPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const Scaffolding* ws = mandatory_non_null_type_cast< const Scaffolding* >(sourceAsset);
	pipelineDepends->addDependency(ws->getScaffoldingClass(), editor::PdfBuild);
	return true;
}

bool ScaffoldingPipeline::buildOutput(
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

Ref< ISerializable > ScaffoldingPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	return nullptr;
}

}
