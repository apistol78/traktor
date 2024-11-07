/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Murmur3.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/PipelineLayoutCache.h"
#include "Render/Vulkan/Private/Utilities.h"

namespace traktor::render
{
	namespace
	{

#if defined(_DEBUG)

const wchar_t* c_descriptorTypes[] =
{
	L"VK_DESCRIPTOR_TYPE_SAMPLER",
	L"VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",
	L"VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE",
	L"VK_DESCRIPTOR_TYPE_STORAGE_IMAGE",
	L"VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER",
	L"VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER",
	L"VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER",
	L"VK_DESCRIPTOR_TYPE_STORAGE_BUFFER",
	L"VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC",
	L"VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC",
	L"VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT"
};

std::wstring describe(const VkDescriptorSetLayoutCreateInfo& dlci)
{
	StringOutputStream ss;
	ss << L"bindingCount = " << dlci.bindingCount << Endl;
	for (uint32_t i = 0; i < dlci.bindingCount; ++i)
	{
		const auto& binding = dlci.pBindings[i];
		ss << L".pBindings[" << i << L"] = {" << Endl;
		ss << L"\t.binding = " << binding.binding << Endl;
		if (binding.descriptorType < sizeof_array(c_descriptorTypes))
			ss << L"\t.descriptorType = " << c_descriptorTypes[binding.descriptorType] << Endl;
		else if (binding.descriptorType == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR)
			ss << L"\t.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR" << Endl;
		ss << L"\t.stageFlags = " << (uint32_t)binding.stageFlags << Endl;
		ss << L"}" << Endl;
	}
	return ss.str();
}

#endif

	}

PipelineLayoutCache::PipelineLayoutCache(Context* context)
:	m_context(context)
{
}

PipelineLayoutCache::~PipelineLayoutCache()
{
	for (auto& it : m_entries)
	{
		vkDestroyDescriptorSetLayout(m_context->getLogicalDevice(), it.second.descriptorSetLayout, 0);
		vkDestroyPipelineLayout(m_context->getLogicalDevice(), it.second.pipelineLayout, 0);
	}

	for (auto& it : m_samplers)
		vkDestroySampler(m_context->getLogicalDevice(), it.second, 0);
}

bool PipelineLayoutCache::get(uint32_t pipelineHash, bool useTargetSize, const VkDescriptorSetLayoutCreateInfo& dlci, VkDescriptorSetLayout& outDescriptorSetLayout, VkPipelineLayout& outPipelineLayout)
{
	auto it = m_entries.find(pipelineHash);
	if (it != m_entries.end())
	{
		outDescriptorSetLayout = it->second.descriptorSetLayout;
		outPipelineLayout = it->second.pipelineLayout;

#if defined(_DEBUG)
		std::wstring d = describe(dlci);
		if (d != it->second.debug)
		{
			log::error << L"Descriptor layout mismatch;" << Endl;
			log::error << L"cached:" << Endl;
			log::error << it->second.debug;
			log::error << L"requested:" << Endl;
			log::error << d;
			T_FATAL_ERROR;
		}
#endif
		return true;
	}
	else
	{
		if (vkCreateDescriptorSetLayout(m_context->getLogicalDevice(), &dlci, nullptr, &outDescriptorSetLayout) != VK_SUCCESS)
			return false;

		// Must match order defined in GlslResource.h
		const VkDescriptorSetLayout setLayouts[] =
		{
			outDescriptorSetLayout,
			m_context->getBindlessTexturesSetLayout(),
			m_context->getBindlessImagesSetLayout(),
			m_context->getBindlessBuffersSetLayout()
		};

		VkPushConstantRange pcr = {};
		pcr.offset = 0;
		pcr.size = 4 * sizeof(float);
		pcr.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		VkPipelineLayoutCreateInfo lci = {};
		lci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		lci.setLayoutCount = sizeof_array(setLayouts);
		lci.pSetLayouts = setLayouts;
		lci.pushConstantRangeCount = 0;
		lci.pPushConstantRanges = nullptr;

		if (useTargetSize)
		{
			lci.pushConstantRangeCount = 1;
			lci.pPushConstantRanges = &pcr;
		}

		if (vkCreatePipelineLayout(m_context->getLogicalDevice(), &lci, nullptr, &outPipelineLayout) != VK_SUCCESS)
			return false;

		auto& entry = m_entries[pipelineHash];
		entry.descriptorSetLayout = outDescriptorSetLayout;
		entry.pipelineLayout = outPipelineLayout;
#if defined(_DEBUG)
		entry.debug = describe(dlci);
#endif
		return true;
	}
}

VkSampler PipelineLayoutCache::getSampler(const VkSamplerCreateInfo& sci)
{
	Murmur3 cs;
	cs.begin();
	cs.feedBuffer(&sci, sizeof(sci));
	cs.end();
	
	const uint32_t samplerHash = cs.get();

	auto it = m_samplers.find(samplerHash);
	if (it != m_samplers.end())
		return it->second;

	VkSampler sampler = 0;
	if (vkCreateSampler(m_context->getLogicalDevice(), &sci, nullptr, &sampler) != VK_SUCCESS)
		return 0;

	setObjectDebugName(m_context->getLogicalDevice(), L"Sampler", (uint64_t)sampler, VK_OBJECT_TYPE_SAMPLER);

	m_samplers.insert(samplerHash, sampler);
	return sampler;
}

}
