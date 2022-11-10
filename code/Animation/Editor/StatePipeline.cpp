/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Animation/SimpleAnimationControllerData.h"
#include "Animation/Animation/StateNodeAnimation.h"
#include "Animation/Animation/StateGraph.h"
#include "Animation/Animation/StatePoseControllerData.h"
#include "Animation/Editor/StatePipeline.h"
#include "Editor/IPipelineDepends.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.StatePipeline", 0, StatePipeline, editor::DefaultPipeline)

TypeInfoSet StatePipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< SimpleAnimationControllerData >();
	typeSet.insert< StateNode >();
	typeSet.insert< StateNodeAnimation >();
	typeSet.insert< StateGraph >();
	typeSet.insert< StatePoseControllerData >();
	return typeSet;
}

bool StatePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (const SimpleAnimationControllerData* simpleControllerData = dynamic_type_cast< const SimpleAnimationControllerData* >(sourceAsset))
		pipelineDepends->addDependency(simpleControllerData->getAnimation(), editor::PdfBuild | editor::PdfResource);
	else if (const StateNodeAnimation* state = dynamic_type_cast< const StateNodeAnimation* >(sourceAsset))
		pipelineDepends->addDependency(state->getAnimation().getId(), editor::PdfBuild | editor::PdfResource);
	else if (const StateGraph* stateGraph = dynamic_type_cast< const StateGraph* >(sourceAsset))
	{
		const RefArray< StateNode >& states = stateGraph->getStates();
		for (RefArray< StateNode >::const_iterator i = states.begin(); i != states.end(); ++i)
			pipelineDepends->addDependency(*i);
	}
	else if (const StatePoseControllerData* controllerData = dynamic_type_cast< const StatePoseControllerData* >(sourceAsset))
		pipelineDepends->addDependency(controllerData->getStateGraph(), editor::PdfBuild);

	return true;
}

	}
}
