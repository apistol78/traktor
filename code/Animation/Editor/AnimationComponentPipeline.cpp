/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/AnimatedMeshComponentData.h"
#include "Animation/IPoseControllerData.h"
#include "Animation/SkeletonComponentData.h"
#include "Animation/Editor/AnimationComponentPipeline.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Editor/IPipelineDepends.h"
#include "World/EntityData.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.AnimationComponentPipeline", 0, AnimationComponentPipeline, editor::IPipeline)

bool AnimationComponentPipeline::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void AnimationComponentPipeline::destroy()
{
}

TypeInfoSet AnimationComponentPipeline::getAssetTypes() const
{
	return makeTypeInfoSet<
		AnimatedMeshComponentData,
		SkeletonComponentData
	>();
}

uint32_t AnimationComponentPipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
}

bool AnimationComponentPipeline::shouldCache() const
{
	return false;
}

bool AnimationComponentPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (const AnimatedMeshComponentData* meshComponentData = dynamic_type_cast< const AnimatedMeshComponentData* >(sourceAsset))
	{
		pipelineDepends->addDependency(meshComponentData->getMesh(), editor::PdfBuild | editor::PdfResource);
	}
	else if (const SkeletonComponentData* skeletonComponentData = dynamic_type_cast< const SkeletonComponentData* >(sourceAsset))
	{
		pipelineDepends->addDependency(skeletonComponentData->getSkeleton(), editor::PdfBuild | editor::PdfResource);
		pipelineDepends->addDependency(skeletonComponentData->getPoseControllerData());

		for (auto binding : skeletonComponentData->getBindings())
			pipelineDepends->addDependency(binding.entityData);
	}
	return true;
}

bool AnimationComponentPipeline::buildOutput(
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
	T_FATAL_ERROR;
	return false;
}

Ref< ISerializable > AnimationComponentPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	return DeepClone(sourceAsset).create();
}

}
