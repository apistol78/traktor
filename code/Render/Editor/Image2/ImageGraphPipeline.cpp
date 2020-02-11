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
#include "Render/Editor/Image2/ImgStepAmbientOcclusion.h"
#include "Render/Editor/Image2/ImgStepDirectionalBlur.h"
#include "Render/Editor/Image2/ImgStepShadowProject.h"
#include "Render/Editor/Image2/ImgStepSimple.h"
#include "Render/Editor/Image2/ImgTargetSet.h"
#include "Render/Editor/Image2/ImgTexture.h"
#include "Render/Image2/AmbientOcclusionData.h"
#include "Render/Image2/DirectionalBlurData.h"
#include "Render/Image2/ImageGraphData.h"
#include "Render/Image2/ImagePassData.h"
#include "Render/Image2/ImageTargetSetData.h"
#include "Render/Image2/ShadowProjectData.h"
#include "Render/Image2/SimpleData.h"

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
				if (auto ambientOcclusionStep = dynamic_type_cast< const ImgStepAmbientOcclusion* >(step))
					pipelineDepends->addDependency(ambientOcclusionStep->m_shader, editor::PdfBuild | editor::PdfResource);
				else if (auto directionalBlurStep = dynamic_type_cast< const ImgStepDirectionalBlur* >(step))
					pipelineDepends->addDependency(directionalBlurStep->m_shader, editor::PdfBuild | editor::PdfResource);
				else if (auto shadowProjectStep = dynamic_type_cast< const ImgStepShadowProject* >(step))
					pipelineDepends->addDependency(shadowProjectStep->m_shader, editor::PdfBuild | editor::PdfResource);
				else if (auto simpleStep = dynamic_type_cast< const ImgStepSimple* >(step))
					pipelineDepends->addDependency(simpleStep->m_shader, editor::PdfBuild | editor::PdfResource);
			}
		}
		else if (auto texture = dynamic_type_cast< const ImgTexture* >(node))
			pipelineDepends->addDependency(texture->m_texture, editor::PdfBuild | editor::PdfResource);
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

	log::info << L"Compiled image graph sequence:" << Endl;
	for (int32_t i = 0; i < (int32_t)nodes.size(); ++i)
		log::info << i << L". " << type_name(nodes[i]) << Endl;

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
			passData->m_output = targetSet->getTargetSetId();
			convertAssetPassToSteps(asset, pass, passData->m_steps);
			data->m_passes.push_back(passData);
		}
		else if (auto targetSet = dynamic_type_cast< const ImgTargetSet* >(nodes[i]))
		{
			// \tbd Since these targets are "local" to this render graph
			// we should rename these with a globally unique identifier
			// so it won't clash with other image graphs.
			Ref< ImageTargetSetData > targetSetData = new ImageTargetSetData();
			targetSetData->m_targetSetId = targetSet->getTargetSetId();
			for (int32_t i = 0; i < targetSet->getTextureCount(); ++i)
				targetSetData->m_textureIds[i] = targetSet->getTargetSetId() + L"/" + targetSet->getTextureId(i);
			targetSetData->m_targetSetDesc = targetSet->getRenderGraphTargetSetDesc();
			data->m_targetSets.push_back(targetSetData);
		}
		else if (auto texture = dynamic_type_cast< const ImgTexture* >(nodes[i]))
		{
			// \tbd Add texture reference to image graph data.
		}
	}

	// Reverse order of passes, not strictly necessary
	// but might improve robustness of render graph pass sorting.
	std::reverse(
		data->m_passes.begin(),
		data->m_passes.end()
	);

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
		if (auto ambientOcclusionStep = dynamic_type_cast< const ImgStepAmbientOcclusion* >(step))
		{
			Ref< AmbientOcclusionData > stepData = new AmbientOcclusionData();
			stepData->m_shader = ambientOcclusionStep->m_shader;
			
			std::set< std::wstring > inputs;
			step->getInputs(inputs);
			for (const auto& input : inputs)
			{
				const InputPin* inputPin = pass->findInputPin(input);
				T_FATAL_ASSERT(inputPin != nullptr);

				const OutputPin* sourcePin = asset->findSourcePin(inputPin);
				if (!sourcePin)
				{
					log::error << L"Image graph pipeline failed; input \"" << input << L"\" not connected." << Endl;
					return false;
				}

				if (auto inputNode = dynamic_type_cast< const ImgInput* >(sourcePin->getNode()))
				{
					// Reading texture from input texture.
					auto& sourceData = stepData->m_sources.push_back();
					sourceData.textureId = inputNode->getTextureId();
					sourceData.parameter = input;
				}
				else if (auto targetSetNode = dynamic_type_cast< const ImgTargetSet* >(sourcePin->getNode()))
				{
					// Reading texture from transient target set.
					auto& sourceData = stepData->m_sources.push_back();
					sourceData.textureId = targetSetNode->getTargetSetId() + L"/" + sourcePin->getName();
					sourceData.parameter = input;
				}
				else if (auto textureNode = dynamic_type_cast< const ImgTexture* >(sourcePin->getNode()))
				{
					// Reading texture resource.
					auto& sourceData = stepData->m_sources.push_back();
					sourceData.textureId = textureNode->getId().format();
					sourceData.parameter = input;
				}
				else
				{
					log::error << L"Image graph pipeline failed; input \"" << input << L"\" connected to incorrect node." << Endl;
					return false;
				}
			}

			outSteps.push_back(stepData);
		}
		else if (auto directionalBlurStep = dynamic_type_cast< const ImgStepDirectionalBlur* >(step))
		{
			Ref< DirectionalBlurData > stepData = new DirectionalBlurData();
			stepData->m_blurType = (DirectionalBlurData::BlurType)directionalBlurStep->m_blurType;
			stepData->m_direction = directionalBlurStep->m_direction;
			stepData->m_taps = directionalBlurStep->m_taps;
			stepData->m_shader = directionalBlurStep->m_shader;

			std::set< std::wstring > inputs;
			step->getInputs(inputs);
			for (const auto& input : inputs)
			{
				const InputPin* inputPin = pass->findInputPin(input);
				T_FATAL_ASSERT(inputPin != nullptr);

				const OutputPin* sourcePin = asset->findSourcePin(inputPin);
				if (!sourcePin)
				{
					log::error << L"Image graph pipeline failed; input \"" << input << L"\" not connected." << Endl;
					return false;
				}

				if (auto inputNode = dynamic_type_cast< const ImgInput* >(sourcePin->getNode()))
				{
					// Reading texture from input texture.
					auto& sourceData = stepData->m_sources.push_back();
					sourceData.textureId = inputNode->getTextureId();
					sourceData.parameter = input;
				}
				else if (auto targetSetNode = dynamic_type_cast< const ImgTargetSet* >(sourcePin->getNode()))
				{
					// Reading texture from transient target set.
					auto& sourceData = stepData->m_sources.push_back();
					sourceData.textureId = targetSetNode->getTargetSetId() + L"/" + sourcePin->getName();
					sourceData.parameter = input;
				}
				else if (auto textureNode = dynamic_type_cast< const ImgTexture* >(sourcePin->getNode()))
				{
					// Reading texture resource.
					auto& sourceData = stepData->m_sources.push_back();
					sourceData.textureId = textureNode->getId().format();
					sourceData.parameter = input;
				}
				else
				{
					log::error << L"Image graph pipeline failed; input \"" << input << L"\" connected to incorrect node." << Endl;
					return false;
				}
			}

			outSteps.push_back(stepData);
		}
		else if (auto shadowProjectStep = dynamic_type_cast< const ImgStepShadowProject* >(step))
		{
			Ref< ShadowProjectData > stepData = new ShadowProjectData();
			stepData->m_shader = shadowProjectStep->m_shader;

			std::set< std::wstring > inputs;
			step->getInputs(inputs);
			for (const auto& input : inputs)
			{
				const InputPin* inputPin = pass->findInputPin(input);
				T_FATAL_ASSERT(inputPin != nullptr);

				const OutputPin* sourcePin = asset->findSourcePin(inputPin);
				if (!sourcePin)
				{
					log::error << L"Image graph pipeline failed; input \"" << input << L"\" not connected." << Endl;
					return false;
				}

				if (auto inputNode = dynamic_type_cast< const ImgInput* >(sourcePin->getNode()))
				{
					// Reading texture from input texture.
					auto& sourceData = stepData->m_sources.push_back();
					sourceData.textureId = inputNode->getTextureId();
					sourceData.parameter = input;
				}
				else if (auto targetSetNode = dynamic_type_cast< const ImgTargetSet* >(sourcePin->getNode()))
				{
					// Reading texture from transient target set.
					auto& sourceData = stepData->m_sources.push_back();
					sourceData.textureId = targetSetNode->getTargetSetId() + L"/" + sourcePin->getName();
					sourceData.parameter = input;
				}
				else if (auto textureNode = dynamic_type_cast< const ImgTexture* >(sourcePin->getNode()))
				{
					// Reading texture resource.
					auto& sourceData = stepData->m_sources.push_back();
					sourceData.textureId = textureNode->getId().format();
					sourceData.parameter = input;
				}
				else
				{
					log::error << L"Image graph pipeline failed; input \"" << input << L"\" connected to incorrect node." << Endl;
					return false;
				}
			}

			outSteps.push_back(stepData);
		}
		else if (auto simpleStep = dynamic_type_cast< const ImgStepSimple* >(step))
		{
			Ref< SimpleData > stepData = new SimpleData();
			stepData->m_shader = simpleStep->m_shader;
			
			std::set< std::wstring > inputs;
			step->getInputs(inputs);
			for (const auto& input : inputs)
			{
				const InputPin* inputPin = pass->findInputPin(input);
				T_FATAL_ASSERT(inputPin != nullptr);

				const OutputPin* sourcePin = asset->findSourcePin(inputPin);
				if (!sourcePin)
				{
					log::error << L"Image graph pipeline failed; input \"" << input << L"\" not connected." << Endl;
					return false;
				}

				if (auto inputNode = dynamic_type_cast< const ImgInput* >(sourcePin->getNode()))
				{
					// Reading texture from input texture.
					auto& sourceData = stepData->m_sources.push_back();
					sourceData.textureId = inputNode->getTextureId();
					sourceData.parameter = input;
				}
				else if (auto targetSetNode = dynamic_type_cast< const ImgTargetSet* >(sourcePin->getNode()))
				{
					// Reading texture from transient target set.
					auto& sourceData = stepData->m_sources.push_back();
					sourceData.textureId = targetSetNode->getTargetSetId() + L"/" + sourcePin->getName();
					sourceData.parameter = input;
				}
				else if (auto textureNode = dynamic_type_cast< const ImgTexture* >(sourcePin->getNode()))
				{
					// Reading texture resource.
					auto& sourceData = stepData->m_sources.push_back();
					sourceData.textureId = textureNode->getId().format();
					sourceData.parameter = input;
				}
				else
				{
					log::error << L"Image graph pipeline failed; input \"" << input << L"\" connected to incorrect node." << Endl;
					return false;
				}
			}

			outSteps.push_back(stepData);
		}
	}
	return true;
}

	}
}
