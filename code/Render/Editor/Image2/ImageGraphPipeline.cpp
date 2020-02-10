#include "Core/Log/Log.h"
#include "Database/Instance.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineBuilder.h"
#include "Render/Editor/GraphTraverse.h"
#include "Render/Editor/Image2/ImageGraphAsset.h"
#include "Render/Editor/Image2/ImageGraphPipeline.h"
#include "Render/Editor/Image2/ImgInput.h"
#include "Render/Editor/Image2/ImgOutput.h"
#include "Render/Editor/Image2/ImgPass.h"
#include "Render/Editor/Image2/ImgStepSimple.h"
#include "Render/Editor/Image2/ImgTargetSet.h"
#include "Render/Image2/ImageGraphData.h"
#include "Render/Image2/ImagePassData.h"
#include "Render/Image2/ImageTargetSetData.h"
#include "Render/Image2/SimpleImageStepData.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageGraphPipeline", 0, ImageGraphPipeline, editor::IPipeline)

bool ImageGraphPipeline::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void ImageGraphPipeline::destroy()
{
}

TypeInfoSet ImageGraphPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< ImageGraphAsset >();
}

bool ImageGraphPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const ImageGraphAsset* asset = mandatory_non_null_type_cast< const ImageGraphAsset* >(sourceAsset);

	// Find output node in graph.
	RefArray< ImgOutput > outputs;
	asset->findNodesOf< ImgOutput >(outputs);
	if (outputs.size() != 1)
	{
		log::error << L"Image graph pipeline failed; graph must only contain one output node." << Endl;
		return false;
	}

	// Traverse nodes starting from output node; collect
	// all resources used by image graph.
	GraphTraverse(asset, outputs.front()).preorder([&](const Node* node)
	{
		if (auto pass = dynamic_type_cast< const ImgPass* >(node))
		{
			for (auto step : pass->getSteps())
			{
				if (auto simple = dynamic_type_cast< const ImgStepSimple* >(step))
					pipelineDepends->addDependency(simple->m_shader, editor::PdfBuild | editor::PdfResource);
			}
		}
		return true;
	});

	return true;
}

bool ImageGraphPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::IPipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	const ImageGraphAsset* asset = mandatory_non_null_type_cast< const ImageGraphAsset* >(sourceAsset);
	
	// Find output node in graph.
	RefArray< ImgOutput > outputs;
	asset->findNodesOf< ImgOutput >(outputs);
	if (outputs.size() != 1)
	{
		log::error << L"Image graph pipeline failed; graph must only contain one output node." << Endl;
		return false;
	}

	// Collect nodes starting from output node.
	RefArray< const Node > nodes;
	GraphTraverse(asset, outputs.front()).preorder([&](const Node* node)
	{
		nodes.push_back(node);
		return true;
	});
	T_ASSERT(is_a< ImgOutput >(nodes.front()));
	nodes.pop_front();

	// Create output resource.
	Ref< ImageGraphData > data = new ImageGraphData();

	// First node in array is the "main" pass of the output image graph.
	T_ASSERT(is_a< ImgPass >(nodes.front()));
	convertAssetPassToSteps(
		asset,
		mandatory_non_null_type_cast< const ImgPass* >(nodes.front()),
		data->m_steps
	);

	// Convert rest of nodes.
	for (size_t i = 1; i < nodes.size(); ++i)
	{
		if (auto pass = dynamic_type_cast< const ImgPass* >(nodes[i]))
		{
			AlignedVector< const InputPin* > destinationPins;
			asset->findDestinationPins(
				pass->getOutputPin(0),
				destinationPins
			);
			if (destinationPins.size() != 1)
			{
				log::error << L"Image graph pipeline failed; pass output only be connected to exactly one output node." << Endl;
				return false;
			}

			auto targetSet = dynamic_type_cast< const ImgTargetSet* >(destinationPins[0]->getNode());
			if (!targetSet)
			{
				log::error << L"Image graph pipeline failed; pass output must be connected to a target node." << Endl;
				return false;
			}

			Ref< ImagePassData > passData = new ImagePassData();
			passData->m_output = targetSet->getTargetSetDesc().id;
			convertAssetPassToSteps(asset, pass, passData->m_steps);
			data->m_passes.push_back(passData);
		}
		else if (auto targetSet = dynamic_type_cast< const ImgTargetSet* >(nodes[i]))
		{
			// \tbd Since these targets are "local" to this render graph
			// we should rename these with a globally unique identifier
			// so it won't clash with other image graphs.
			Ref< ImageTargetSetData > targetSetData = new ImageTargetSetData();
			targetSetData->m_targetSetDesc = targetSet->getTargetSetDesc();
			data->m_targetSets.push_back(targetSetData);
		}
	}

	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(outputPath, outputGuid);
	if (!outputInstance)
		return false;

	outputInstance->setObject(data);

	if (!outputInstance->commit())
		return false;

	return true;
}

Ref< ISerializable > ImageGraphPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	T_FATAL_ERROR;
	return nullptr;
}

bool ImageGraphPipeline::convertAssetPassToSteps(const ImageGraphAsset* asset, const ImgPass* pass, RefArray< IImageStepData >& outSteps) const
{
	for (auto step : pass->getSteps())
	{
		if (auto simpleStep = dynamic_type_cast< const ImgStepSimple* >(step))
		{
			Ref< SimpleImageStepData > simpleData = new SimpleImageStepData();
			simpleData->m_shader = simpleStep->m_shader;
			
			for (const auto& parameter : simpleStep->m_parameters)
			{
				const InputPin* inputPin = pass->findInputPin(parameter);
				T_FATAL_ASSERT(inputPin != nullptr);

				const OutputPin* sourcePin = asset->findSourcePin(inputPin);
				if (!sourcePin)
				{
					log::error << L"Image graph pipeline failed; input \"" << parameter << L"\" not connected." << Endl;
					return false;
				}

				if (auto input = dynamic_type_cast< const ImgInput* >(sourcePin->getNode()))
				{
					// Reading texture from input texture.
					auto& sourceData = simpleData->m_sources.push_back();
					sourceData.textureId = input->getTextureId();
					sourceData.parameter = parameter;
				}
				else if (auto targetSet = dynamic_type_cast< const ImgTargetSet* >(sourcePin->getNode()))
				{
					// Reading texture from transient target set.
					auto& sourceData = simpleData->m_sources.push_back();
					//sourceData.textureId = input->getTextureId();
					sourceData.parameter = parameter;
				}
				else
				{
					log::error << L"Image graph pipeline failed; input \"" << parameter << L"\" connected to incorrect node." << Endl;
					return false;
				}
			}

			outSteps.push_back(simpleData);
		}
	}
	return true;
}

	}
}
