/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Editor/StatePipeline.h"

#include "Animation/Editor/StateGraph.h"
#include "Animation/Editor/StateGraphCompiler.h"
#include "Animation/Editor/StateNodeAnimation.h"
#include "Editor/IPipelineDepends.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.StatePipeline", 0, StatePipeline, editor::DefaultPipeline)

TypeInfoSet StatePipeline::getAssetTypes() const
{
	return makeTypeInfoSet<
		StateGraph,
		StateNodeAnimation >();
}

bool StatePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid) const
{
	if (auto stateGraph = dynamic_type_cast< const StateGraph* >(sourceAsset))
		for (auto state : stateGraph->getStates())
			pipelineDepends->addDependency(state);
	else if (auto state = dynamic_type_cast< const StateNodeAnimation* >(sourceAsset))
		pipelineDepends->addDependency(state->getAnimation(), editor::PdfBuild | editor::PdfResource);

	return true;
}

bool StatePipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::PipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason) const
{
	if (auto stateGraph = dynamic_type_cast< const StateGraph* >(sourceAsset))
	{
		// Compile graph into an optimized runtime graph representation.
		return StateGraphCompiler().compile(stateGraph);
	}
	else
		return editor::DefaultPipeline::buildOutput(
			pipelineBuilder,
			dependencySet,
			dependency,
			sourceInstance,
			sourceAsset,
			outputPath,
			outputGuid,
			buildParams,
			reason);
}

}
