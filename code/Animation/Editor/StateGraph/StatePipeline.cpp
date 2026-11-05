/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Editor/StateGraph/StatePipeline.h"

#include "Animation/Editor/StateGraph/StateGraph.h"
#include "Animation/Editor/StateGraph/StateGraphCompiler.h"
#include "Animation/Editor/StateGraph/StateNodeAnimation.h"
#include "Animation/Editor/StateGraph/StateNodeController.h"
#include "Animation/Editor/StateGraph/StateNodeStack.h"
#include "Animation/IPoseControllerData.h"
#include "Animation/StateGraph/RtStateGraphData.h"
#include "Core/Log/Log.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.StatePipeline", 3, StatePipeline, editor::DefaultPipeline)

TypeInfoSet StatePipeline::getAssetTypes() const
{
	return makeTypeInfoSet<
		StateGraph,
		StateNodeAnimation,
		StateNodeController,
		StateNodeStack >();
}

bool StatePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid) const
{
	if (auto stateGraph = dynamic_type_cast< const StateGraph* >(sourceAsset))
	{
		for (auto state : stateGraph->getStates())
			pipelineDepends->addDependency(state);

		pipelineDepends->addDependency(stateGraph->getPreviewSkeleton(), editor::PdfBuild);
		pipelineDepends->addDependency(stateGraph->getPreviewMesh(), editor::PdfBuild);
	}
	else if (auto state = dynamic_type_cast< const StateNodeAnimation* >(sourceAsset))
	{
		for (const auto& animation : state->getAnimations())
			pipelineDepends->addDependency(animation, editor::PdfBuild | editor::PdfResource);
	}
	else if (auto state = dynamic_type_cast< const StateNodeController* >(sourceAsset))
	{
		pipelineDepends->addDependency(state->getPoseController());
	}
	else if (auto state = dynamic_type_cast< const StateNodeStack* >(sourceAsset))
	{
		for (const auto& layer : state->getLayers())
			pipelineDepends->addDependency(layer.animation, editor::PdfBuild | editor::PdfResource);
	}

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
		Ref< RtStateGraphData > rtg = StateGraphCompiler().compile(stateGraph);
		if (!rtg)
			return false;

		Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(outputPath, outputGuid);
		if (!outputInstance)
		{
			log::error << L"StatePipeline; Unable to create output instance." << Endl;
			return false;
		}

		outputInstance->setObject(rtg);

		if (!outputInstance->commit())
		{
			log::error << L"StatePipeline; Unable to commit output instance." << Endl;
			return false;
		}

		return true;
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
