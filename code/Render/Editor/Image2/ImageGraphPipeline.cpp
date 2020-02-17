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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageGraphPipeline", 1, ImageGraphPipeline, editor::IPipeline)

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
	T_ASSERT(is_a< ImgOutput >(nodes.front()));
	nodes.pop_front();

	// Create output resource.
	Ref< ImageGraphData > data = new ImageGraphData();

	// First node in array is the "main" pass of the output image graph.
	T_ASSERT(is_a< ImgPass >(nodes.front()));
	log::info << L"Pass" << Endl;
	convertAssetPassToSteps(
		asset,
		mandatory_non_null_type_cast< const ImgPass* >(nodes.front()),
		data->m_steps
	);

	// Convert all target sets.
	for (size_t i = 1; i < nodes.size(); ++i)
	{
		if (auto targetSet = dynamic_type_cast< const ImgTargetSet* >(nodes[i]))
		{
			Ref< ImageTargetSetData > tsd = new ImageTargetSetData();
			
			tsd->m_targetSetId = targetSet->getTargetSetId();
			for (int32_t i = 0; i < targetSet->getTextureCount(); ++i)
				tsd->m_textureIds[i] = targetSet->getTargetSetId() + L"/" + targetSet->getTextureId(i);
			tsd->m_targetSetDesc = targetSet->getRenderGraphTargetSetDesc();

			data->m_targetSets.push_back(tsd);
		}
	}

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
			passData->m_name = pass->getName();
			passData->m_outputTargetSet = -1;

			// Find index of output target set.
			int32_t targetSetIndex = 0;
			for (size_t i = 1; i < nodes.size(); ++i)
			{
				if (!is_a< ImgTargetSet >(nodes[i]))
					continue;

				if (targetSet == nodes[i])
				{
					passData->m_outputTargetSet = targetSetIndex;
					break;
				}
				else
					++targetSetIndex;
			}

			// Convert pass's steps.
			convertAssetPassToSteps(asset, pass, passData->m_steps);

			data->m_passes.push_back(passData);
		}
		else if (auto texture = dynamic_type_cast< const ImgTexture* >(nodes[i]))
		{
			// \tbd Add texture reference to image graph data.
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

bool ImageGraphPipeline::convertAssetPassToSteps(const ImageGraphAsset* asset, const ImgPass* pass, RefArray< ImageStepData >& outSteps) const
{
	for (auto step : pass->getSteps())
	{
		Ref< ImageStepData > stepData;

		// Create step data instance.
		if (auto ambientOcclusionStep = dynamic_type_cast< const ImgStepAmbientOcclusion* >(step))
		{
			Ref< AmbientOcclusionData > ao = new AmbientOcclusionData();
			ao->m_shader = ambientOcclusionStep->m_shader;
			stepData = ao;
		}
		else if (auto directionalBlurStep = dynamic_type_cast< const ImgStepDirectionalBlur* >(step))
		{
			Ref< DirectionalBlurData > db = new DirectionalBlurData();
			db->m_shader = directionalBlurStep->m_shader;
			db->m_blurType = (DirectionalBlurData::BlurType)directionalBlurStep->m_blurType;
			db->m_direction = directionalBlurStep->m_direction;
			db->m_taps = directionalBlurStep->m_taps;
			stepData = db;
		}
		else if (auto shadowProjectStep = dynamic_type_cast< const ImgStepShadowProject* >(step))
		{
			Ref< ShadowProjectData > sp = new ShadowProjectData();
			sp->m_shader = shadowProjectStep->m_shader;
			stepData = sp;
		}
		else if (auto simpleStep = dynamic_type_cast< const ImgStepSimple* >(step))
		{
			Ref< SimpleData > s = new SimpleData();
			s->m_shader = simpleStep->m_shader;
			stepData = s;
		}
		else
		{
			log::error << L"Image graph pipeline failed; unable to convert node of \"" << type_name(step) << L"\"." << Endl;
			return false;
		}
		T_FATAL_ASSERT(stepData);

		log::info << L"\t\"" << type_name(stepData) << L"\", shader = \"" << Guid(stepData->m_shader).format() << L"\"." << Endl;

		// Setup input parameters.
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
				log::info << L"\tparameter \"" << sourceData.parameter << L"\" = input \"" << sourceData.textureId << L"\"." << Endl;
			}
			else if (auto targetSetNode = dynamic_type_cast< const ImgTargetSet* >(sourcePin->getNode()))
			{
				// Reading texture from transient target set.
				auto& sourceData = stepData->m_sources.push_back();
				sourceData.textureId = targetSetNode->getTargetSetId() + L"/" + sourcePin->getName();
				sourceData.parameter = input;
				log::info << L"\tparameter \"" << sourceData.parameter << L"\" = transient target \"" << sourceData.textureId << L"\"." << Endl;
			}
			else if (auto textureNode = dynamic_type_cast< const ImgTexture* >(sourcePin->getNode()))
			{
				// Reading texture resource.
				auto& sourceData = stepData->m_sources.push_back();
				sourceData.textureId = textureNode->getId().format();
				sourceData.parameter = input;
				log::info << L"\tparameter \"" << sourceData.parameter << L"\" = texture resource \"" << sourceData.textureId << L"\"." << Endl;
			}
			else
			{
				log::error << L"Image graph pipeline failed; input \"" << input << L"\" connected to incorrect node." << Endl;
				return false;
			}
		}

		outSteps.push_back(stepData);
	}
	return true;
}

	}
}
