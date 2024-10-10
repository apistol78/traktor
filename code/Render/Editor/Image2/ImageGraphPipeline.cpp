/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Serialization/DeepHash.h"
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
#include "Render/Editor/Image2/ImgStepCompute.h"
#include "Render/Editor/Image2/ImgStepDirectionalBlur.h"
#include "Render/Editor/Image2/ImgStepShadowProject.h"
#include "Render/Editor/Image2/ImgStepSimple.h"
#include "Render/Editor/Image2/ImgStructBuffer.h"
#include "Render/Editor/Image2/ImgTargetSet.h"
#include "Render/Editor/Image2/ImgTexture.h"
#include "Render/Image2/AmbientOcclusionData.h"
#include "Render/Image2/ComputeData.h"
#include "Render/Image2/DirectionalBlurData.h"
#include "Render/Image2/ImageGraphData.h"
#include "Render/Image2/ImagePassData.h"
#include "Render/Image2/ImageStructBufferData.h"
#include "Render/Image2/ImageTargetSetData.h"
#include "Render/Image2/ImageTextureData.h"
#include "Render/Image2/ShadowProjectData.h"
#include "Render/Image2/SimpleData.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageGraphPipeline", 15, ImageGraphPipeline, editor::IPipeline)

bool ImageGraphPipeline::create(const editor::IPipelineSettings* settings, db::Database* database)
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

bool ImageGraphPipeline::shouldCache() const
{
	return false;
}

uint32_t ImageGraphPipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
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
	RefArray< ImgOutput > outputs = asset->findNodesOf< ImgOutput >();
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
				else if (auto computeStep = dynamic_type_cast< const ImgStepCompute* >(step))
					pipelineDepends->addDependency(computeStep->m_shader, editor::PdfBuild | editor::PdfResource);
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
	const ImageGraphAsset* asset = mandatory_non_null_type_cast< const ImageGraphAsset* >(sourceAsset);
	
	// Find output node in graph.
	RefArray< ImgOutput > outputs = asset->findNodesOf< ImgOutput >();
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

	auto rootPass = mandatory_non_null_type_cast< const ImgPass* >(nodes.front());

	// Create output resource.
	Ref< ImageGraphData > data = new ImageGraphData();
	data->m_name = rootPass->getName();

	// First node in array is the "main" pass of the output image graph.
	T_ASSERT(is_a< ImgPass >(nodes.front()));
	convertAssetPassToSteps(
		asset,
		rootPass,
		data->m_steps
	);

	// Convert all textures and target sets.
	for (size_t i = 1; i < nodes.size(); ++i)
	{
		if (auto sbufferNode = dynamic_type_cast< const ImgStructBuffer* >(nodes[i]))
		{
			Ref< ImageStructBufferData > sbd = new ImageStructBufferData();

			sbd->m_id = sbufferNode->getId().format();

			if (sbufferNode->getPersistent())
				sbd->m_persistentHandle = Guid::create().format();

			sbd->m_bufferSize = sbufferNode->m_elementCount * sbufferNode->m_elementSize;

			data->m_sbuffers.push_back(sbd);
		}
		else if (auto textureNode = dynamic_type_cast< const ImgTexture* >(nodes[i]))
		{
			Ref< ImageTextureData > td = new ImageTextureData();

			td->m_textureId = textureNode->getId().format();
			td->m_texture = textureNode->m_texture;

			data->m_textures.push_back(td);
		}		
		else if (auto targetSetNode = dynamic_type_cast< const ImgTargetSet* >(nodes[i]))
		{
			Ref< ImageTargetSetData > tsd = new ImageTargetSetData();
			
			tsd->m_targetSetId = targetSetNode->getTargetSetId();

			if (targetSetNode->getPersistent())
				tsd->m_persistentHandle = Guid::create().format();

			for (int32_t i = 0; i < targetSetNode->getTextureCount(); ++i)
				tsd->m_textureIds[i] = targetSetNode->getTargetSetId() + L"/" + targetSetNode->getTextureId(i);

			tsd->m_targetSetDesc = targetSetNode->getRenderGraphTargetSetDesc();

			data->m_targetSets.push_back(tsd);
		}
	}

	// Convert rest of nodes.
	for (size_t i = 1; i < nodes.size(); ++i)
	{
		if (auto pass = dynamic_type_cast< const ImgPass* >(nodes[i]))
		{
			AlignedVector< const InputPin* > destinationPins = asset->findDestinationPins(pass->getOutputPin(0));
			if (destinationPins.size() != 1)
			{
				log::error << L"Image graph pipeline failed; pass output only be connected to exactly one output node." << Endl;
				return false;
			}

			if (auto targetSet = dynamic_type_cast< const ImgTargetSet* >(destinationPins[0]->getNode()))
			{
				Ref< ImagePassData > passData = new ImagePassData();
				passData->m_name = pass->getName();
				passData->m_clear = pass->getClear();

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
			else if (auto sbuffer = dynamic_type_cast< const ImgStructBuffer* >(destinationPins[0]->getNode()))
			{
				Ref< ImagePassData > passData = new ImagePassData();
				passData->m_name = pass->getName();
				passData->m_clear = pass->getClear();

				// Find index of output sbuffer.
				int32_t sbufferIndex = 0;
				for (size_t i = 1; i < nodes.size(); ++i)
				{
					if (!is_a< ImgStructBuffer >(nodes[i]))
						continue;

					if (sbuffer == nodes[i])
					{
						passData->m_outputSBuffer = sbufferIndex;
						break;
					}
					else
						++sbufferIndex;
				}

				// Convert pass's steps.
				convertAssetPassToSteps(asset, pass, passData->m_steps);

				data->m_passes.push_back(passData);
			}
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

Ref< ISerializable > ImageGraphPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	T_FATAL_ERROR;
	return nullptr;
}

bool ImageGraphPipeline::convertAssetPassToSteps(const ImageGraphAsset* asset, const ImgPass* pass, RefArray< ImagePassStepData >& outOpData) const
{
	log::info << L"Pass \"" << pass->getName() << L"\" begin" << Endl;

	// Log output.
	{
		const OutputPin* outputPin = pass->getOutputPin(0);
		T_FATAL_ASSERT(outputPin != nullptr);

		const AlignedVector< const InputPin* > destinationPins = asset->findDestinationPins(outputPin);
		if (destinationPins.size() != 1)
		{
			log::error << L"Image graph pipeline failed; pass output not connected properly." << Endl;
			return false;
		}

		if (auto targetSetNode = dynamic_type_cast< const ImgTargetSet* >(destinationPins.front()->getNode()))
		{
			const std::wstring textureId = targetSetNode->getTargetSetId() + L"/" + outputPin->getName();
			if (!targetSetNode->getPersistent())
				log::info << L"\tOutput into transient target \"" << textureId << L"\"." << Endl;
			else
				log::info << L"\tOutput into persistent target \"" << textureId << L"\"." << Endl;
		}
		else if (auto sbufferNode = dynamic_type_cast< const ImgStructBuffer* >(destinationPins.front()->getNode()))
		{
			const std::wstring sbufferId = sbufferNode->getId().format() + L"/" + outputPin->getName();
			if (!sbufferNode->getPersistent())
				log::info << L"\tOutput into transient sbuffer \"" << sbufferId << L"\"." << Endl;
			else
				log::info << L"\tOutput into persistent sbuffer \"" << sbufferId << L"\"." << Endl;
		}
		else if (auto outputNode = dynamic_type_cast< const ImgOutput* >(destinationPins.front()->getNode()))
		{
			log::info << L"\tOutput into back buffer." << Endl;
		}
		else
		{
			log::error << L"Image graph pipeline failed; pass output not connected to a target." << Endl;
			return false;
		}
	}

	for (auto step : pass->getSteps())
	{
		Ref< ImagePassStepData > opData;

		// Create step data instance.
		if (auto ambientOcclusionStep = dynamic_type_cast< const ImgStepAmbientOcclusion* >(step))
		{
			Ref< AmbientOcclusionData > ao = new AmbientOcclusionData();
			ao->m_shader = ambientOcclusionStep->m_shader;
			opData = ao;
		}
		else if (auto computeStep = dynamic_type_cast< const ImgStepCompute* >(step))
		{
			Ref< ComputeData > c = new ComputeData();
			c->m_shader = computeStep->m_shader;
			c->m_workSize = computeStep->m_workSize;
			c->m_manualWorkSize[0] = computeStep->m_manualWorkSize[0];
			c->m_manualWorkSize[1] = computeStep->m_manualWorkSize[1];
			c->m_manualWorkSize[2] = computeStep->m_manualWorkSize[2];
			opData = c;
		}
		else if (auto directionalBlurStep = dynamic_type_cast< const ImgStepDirectionalBlur* >(step))
		{
			Ref< DirectionalBlurData > db = new DirectionalBlurData();
			db->m_shader = directionalBlurStep->m_shader;
			db->m_blurType = directionalBlurStep->m_blurType;
			db->m_direction = directionalBlurStep->m_direction;
			db->m_taps = directionalBlurStep->m_taps;
			opData = db;
		}
		else if (auto shadowProjectStep = dynamic_type_cast< const ImgStepShadowProject* >(step))
		{
			Ref< ShadowProjectData > sp = new ShadowProjectData();
			sp->m_shader = shadowProjectStep->m_shader;
			opData = sp;
		}
		else if (auto simpleStep = dynamic_type_cast< const ImgStepSimple* >(step))
		{
			Ref< SimpleData > s = new SimpleData();
			s->m_shader = simpleStep->m_shader;
			opData = s;
		}
		else
		{
			log::error << L"Image graph pipeline failed; unable to convert node of \"" << type_name(step) << L"\"." << Endl;
			return false;
		}
		T_FATAL_ASSERT(opData);

		log::info << L"\tStep \"" << type_name(opData) << L"\", shader = \"" << Guid(opData->m_shader).format() << L"\"." << Endl;

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
				auto& sourceData = opData->m_textureSources.push_back();
				sourceData.id = inputNode->getTextureId();
				sourceData.shaderParameter = input;
				log::info << L"\t\tParameter \"" << sourceData.shaderParameter << L"\" = input \"" << sourceData.id << L"\"." << Endl;
			}
			else if (auto structBufferNode = dynamic_type_cast< const ImgStructBuffer* >(sourcePin->getNode()))
			{
				// Reading struct buffer.
				auto& sourceData = opData->m_sbufferSources.push_back();
				sourceData.id = structBufferNode->getId().format();
				sourceData.shaderParameter = input;
				if (!structBufferNode->getPersistent())
					log::info << L"\t\tParameter \"" << sourceData.shaderParameter << L"\" = transient sbuffer \"" << sourceData.id << L"\"." << Endl;
				else
					log::info << L"\t\tParameter \"" << sourceData.shaderParameter << L"\" = persistent sbuffer \"" << sourceData.id << L"\"." << Endl;
			}
			else if (auto targetSetNode = dynamic_type_cast< const ImgTargetSet* >(sourcePin->getNode()))
			{
				// Reading texture from transient target set.
				auto& sourceData = opData->m_textureSources.push_back();
				sourceData.id = targetSetNode->getTargetSetId() + L"/" + sourcePin->getName();
				sourceData.shaderParameter = input;
				if (!targetSetNode->getPersistent())
					log::info << L"\t\tParameter \"" << sourceData.shaderParameter << L"\" = transient target \"" << sourceData.id << L"\"." << Endl;
				else
					log::info << L"\t\tParameter \"" << sourceData.shaderParameter << L"\" = persistent target \"" << sourceData.id << L"\"." << Endl;
			}
			else if (auto textureNode = dynamic_type_cast< const ImgTexture* >(sourcePin->getNode()))
			{
				// Reading texture resource.
				auto& sourceData = opData->m_textureSources.push_back();
				sourceData.id = textureNode->getId().format();
				sourceData.shaderParameter = input;
				log::info << L"\t\tParameter \"" << sourceData.shaderParameter << L"\" = texture resource \"" << sourceData.id << L"\"." << Endl;
			}
			else
			{
				log::error << L"Image graph pipeline failed; input \"" << input << L"\" connected to incorrect node." << Endl;
				return false;
			}
		}

		outOpData.push_back(opData);
	}

	log::info << L"Pass \"" << pass->getName() << L"\" end" << Endl;
	return true;
}

}
