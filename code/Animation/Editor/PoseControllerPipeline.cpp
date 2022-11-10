/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Animation/StatePoseControllerData.h"
#include "Animation/IK/IKPoseControllerData.h"
#include "Animation/RagDoll/RagDollPoseControllerData.h"
#include "Animation/Editor/PoseControllerPipeline.h"
#include "Editor/IPipelineDepends.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.PoseControllerPipeline", 0, PoseControllerPipeline, editor::DefaultPipeline)

TypeInfoSet PoseControllerPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< StatePoseControllerData >();
	typeSet.insert< IKPoseControllerData >();
	typeSet.insert< RagDollPoseControllerData >();
	return typeSet;
}

bool PoseControllerPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (const StatePoseControllerData* statePoseControllerData = dynamic_type_cast< const StatePoseControllerData* >(sourceAsset))
		pipelineDepends->addDependency(statePoseControllerData->getStateGraph(), editor::PdfBuild | editor::PdfResource);
	else if (const IKPoseControllerData* ikPoseContollerData = dynamic_type_cast< const IKPoseControllerData* >(sourceAsset))
		pipelineDepends->addDependency(ikPoseContollerData->getNeutralPoseController());
	else if (const RagDollPoseControllerData* ragDollPoseContollerData = dynamic_type_cast< const RagDollPoseControllerData* >(sourceAsset))
		pipelineDepends->addDependency(ragDollPoseContollerData->getTrackPoseController());
	return true;
}

	}
}
