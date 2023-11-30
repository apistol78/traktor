/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <functional>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Serialization/DeepClone.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Sound/Editor/SoundCategory.h"
#include "Sound/Editor/Processor/GraphAsset.h"
#include "Sound/Editor/Processor/GraphPipeline.h"
#include "Sound/Processor/Edge.h"
#include "Sound/Processor/Graph.h"
#include "Sound/Processor/GraphResource.h"
#include "Sound/Processor/Node.h"
#include "Sound/Processor/OutputPin.h"
#include "Sound/Processor/Nodes/Custom.h"
#include "Sound/Processor/Nodes/Source.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.GraphPipeline", 0, GraphPipeline, editor::DefaultPipeline)

TypeInfoSet GraphPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< GraphAsset >();
}

bool GraphPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const GraphAsset* graphAsset = checked_type_cast< const GraphAsset*, false >(sourceAsset);
	if (!graphAsset->getGraph())
		return false;

	Ref< const SoundCategory > category = pipelineDepends->getObjectReadOnly< SoundCategory >(graphAsset->getCategory());
	if (category)
		pipelineDepends->addDependency(graphAsset->getCategory(), editor::PdfUse);

	const auto& nodes = graphAsset->getGraph()->getNodes();
	for (const auto node : nodes)
	{
		if (auto source = dynamic_type_cast< const Source* >(node))
			pipelineDepends->addDependency(source->getSound().getId(), editor::PdfBuild);
		else if (auto custom = dynamic_type_cast< const Custom* >(node))
			pipelineDepends->addDependency(custom->getClass().getId(), editor::PdfBuild);
	}

	return true;
}

bool GraphPipeline::buildOutput(
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
	const GraphAsset* graphAsset = checked_type_cast< const GraphAsset*, false >(sourceAsset);

	bool categorized = false;
	std::wstring configurationId;
	float gain = 0.0f;
	float range = 0.0f;

	Ref< const SoundCategory > category = pipelineBuilder->getObjectReadOnly< SoundCategory >(graphAsset->getCategory());

	if (category)
		configurationId = category->getConfigurationId();

	while (category)
	{
		categorized = true;
		gain += category->getGain();
		range = std::max(range, category->getRange());
		category = pipelineBuilder->getObjectReadOnly< SoundCategory >(category->getParent());
	}

	if (categorized)
	{
		log::info << L"Category gain " << gain << L" dB" << Endl;
		log::info << L"         range " << range << Endl;
	}
	else
		log::warning << L"Uncategorized sound \"" << sourceInstance->getName() << L"\"" << Endl;

	Ref< Graph > graph = DeepClone(graphAsset->getGraph()).create< Graph >();
	if (!graph)
	{
		log::error << L"Unable to clone processor graph." << Endl;
		return false;
	}

	Ref< GraphResource > graphResource = new GraphResource(
		configurationId,
		gain,
		range,
		graph
	);

	return editor::DefaultPipeline::buildOutput(
		pipelineBuilder,
		dependencySet,
		dependency,
		sourceInstance,
		graphResource,
		outputPath,
		outputGuid,
		buildParams,
		reason
	);
}

}
