/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Editor/Stack/StackPipeline.h"

#include "Animation/Editor/Stack/Stack.h"
#include "Animation/Editor/Stack/StackCompiler.h"
#include "Animation/Editor/Stack/StackLayer.h"
#include "Animation/IPoseControllerData.h"
#include "Animation/Stack/RtStackData.h"
#include "Core/Log/Log.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.StackPipeline", 0, StackPipeline, editor::DefaultPipeline)

TypeInfoSet StackPipeline::getAssetTypes() const
{
	return makeTypeInfoSet<
		Stack,
		StackLayer >();
}

bool StackPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid) const
{
	if (auto stack = dynamic_type_cast< const Stack* >(sourceAsset))
	{
		for (auto layer : stack->getLayers())
			pipelineDepends->addDependency(layer);

		pipelineDepends->addDependency(stack->getPreviewSkeleton(), editor::PdfBuild);
		pipelineDepends->addDependency(stack->getPreviewMesh(), editor::PdfBuild);
	}
	else if (auto layer = dynamic_type_cast< const StackLayer* >(sourceAsset))
	{
		pipelineDepends->addDependency(layer->getAnimation(), editor::PdfBuild | editor::PdfResource);
		pipelineDepends->addDependency(layer->getPoseController());
	}

	return true;
}

bool StackPipeline::buildOutput(
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
	if (auto stack = dynamic_type_cast< const Stack* >(sourceAsset))
	{
		Ref< RtStackData > rtsd = StackCompiler().compile(stack);
		if (!rtsd)
			return false;

		Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(outputPath, outputGuid);
		if (!outputInstance)
		{
			log::error << L"StackPipeline; Unable to create output instance." << Endl;
			return false;
		}

		outputInstance->setObject(rtsd);

		if (!outputInstance->commit())
		{
			log::error << L"StackPipeline; Unable to commit output instance." << Endl;
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
