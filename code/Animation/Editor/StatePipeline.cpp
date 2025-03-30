/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Animation/AnimationGraphPoseControllerData.h"
#include "Animation/Animation/SimpleAnimationControllerData.h"
#include "Animation/Editor/StateGraph.h"
#include "Animation/Editor/StateNode.h"
#include "Animation/Editor/StatePipeline.h"
#include "Editor/IPipelineDepends.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.StatePipeline", 0, StatePipeline, editor::DefaultPipeline)

TypeInfoSet StatePipeline::getAssetTypes() const
{
	return makeTypeInfoSet<
		SimpleAnimationControllerData,
		StateGraph,
		StateNode,
		AnimationGraphPoseControllerData
	>();
}

bool StatePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (auto simpleControllerData = dynamic_type_cast< const SimpleAnimationControllerData* >(sourceAsset))
		pipelineDepends->addDependency(simpleControllerData->getAnimation(), editor::PdfBuild | editor::PdfResource);
	else if (auto stateGraph = dynamic_type_cast< const StateGraph* >(sourceAsset))
	{
		for (auto state : stateGraph->getStates())
			pipelineDepends->addDependency(state);
	}
	else if (auto state = dynamic_type_cast< const StateNode* >(sourceAsset))
		pipelineDepends->addDependency(state->getAnimation(), editor::PdfBuild | editor::PdfResource);
	else if (auto controllerData = dynamic_type_cast< const AnimationGraphPoseControllerData* >(sourceAsset))
		pipelineDepends->addDependency(controllerData->getStateGraph(), editor::PdfBuild);

	return true;
}

}
