/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Editor/PoseControllerPipeline.h"

#include "Animation/Animation/AnimationGraphPoseControllerData.h"
#include "Animation/Animation/RetargetPoseControllerData.h"
#include "Animation/Animation/SimpleAnimationControllerData.h"
#include "Animation/RagDoll/RagDollPoseControllerData.h"
#include "Editor/IPipelineDepends.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.PoseControllerPipeline", 0, PoseControllerPipeline, editor::DefaultPipeline)

TypeInfoSet PoseControllerPipeline::getAssetTypes() const
{
	return makeTypeInfoSet<
		AnimationGraphPoseControllerData,
		RetargetPoseControllerData,
		RagDollPoseControllerData,
		SimpleAnimationControllerData >();
}

bool PoseControllerPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid) const
{
	if (auto statePoseControllerData = dynamic_type_cast< const AnimationGraphPoseControllerData* >(sourceAsset))
		pipelineDepends->addDependency(statePoseControllerData->getStateGraph(), editor::PdfBuild | editor::PdfResource);
	else if (auto retargetPoseControllerData = dynamic_type_cast< const RetargetPoseControllerData* >(sourceAsset))
	{
		pipelineDepends->addDependency(retargetPoseControllerData->getAnimationSkeleton(), editor::PdfBuild | editor::PdfResource);
		pipelineDepends->addDependency(retargetPoseControllerData->getPoseController());
	}
	else if (auto ragDollPoseContollerData = dynamic_type_cast< const RagDollPoseControllerData* >(sourceAsset))
	{
		pipelineDepends->addDependency(ragDollPoseContollerData->getTrackPoseController());
		for (const auto& id : ragDollPoseContollerData->getCollisionGroup())
			pipelineDepends->addDependency(id, editor::PdfBuild | editor::PdfResource);
		for (const auto& id : ragDollPoseContollerData->getCollisionMask())
			pipelineDepends->addDependency(id, editor::PdfBuild | editor::PdfResource);
	}
	else if (auto simpleControllerData = dynamic_type_cast< const SimpleAnimationControllerData* >(sourceAsset))
		pipelineDepends->addDependency(simpleControllerData->getAnimation(), editor::PdfBuild | editor::PdfResource);
	else
		return false;

	return true;
}

}
