/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Animation/Editor/AnimatedMeshComponentPipeline.h"
#include "Animation/AnimatedMeshComponentData.h"
#include "Animation/IPoseControllerData.h"
#include "Editor/IPipelineDepends.h"
#include "World/EntityData.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.AnimatedMeshComponentPipeline", 0, AnimatedMeshComponentPipeline, editor::IPipeline)

bool AnimatedMeshComponentPipeline::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void AnimatedMeshComponentPipeline::destroy()
{
}

TypeInfoSet AnimatedMeshComponentPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< AnimatedMeshComponentData >();
}

uint32_t AnimatedMeshComponentPipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
}

bool AnimatedMeshComponentPipeline::shouldCache() const
{
	return false;
}

bool AnimatedMeshComponentPipeline::buildDependencies(
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
		pipelineDepends->addDependency(meshComponentData->getSkeleton(), editor::PdfBuild | editor::PdfResource);
		pipelineDepends->addDependency(meshComponentData->getPoseControllerData());

		for (auto binding : meshComponentData->getBindings())
			pipelineDepends->addDependency(binding.entityData);
	}
	return true;
}

bool AnimatedMeshComponentPipeline::buildOutput(
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

Ref< ISerializable > AnimatedMeshComponentPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	return DeepClone(sourceAsset).create();
}

	}
}
