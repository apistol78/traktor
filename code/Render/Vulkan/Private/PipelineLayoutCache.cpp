/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Vulkan/Private/PipelineLayoutCache.h"

#include "Core/Log/Log.h"
#include "Core/Misc/Murmur3.h"
#include "Core/Thread/Acquire.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/Utilities.h"

namespace traktor::render
{
namespace
{

bool equal(const VkDescriptorSetLayoutBinding& lh, const VkDescriptorSetLayoutBinding& rh)
{
	return lh.binding == rh.binding &&
		lh.descriptorType == rh.descriptorType &&
		lh.descriptorCount == rh.descriptorCount &&
		lh.stageFlags == rh.stageFlags &&
		lh.pImmutableSamplers == rh.pImmutableSamplers;
}

bool equal(const AlignedVector< VkDescriptorSetLayoutBinding >& bindings, const VkDescriptorSetLayoutCreateInfo& dlci)
{
	if (bindings.size() != dlci.bindingCount)
		return false;
	for (uint32_t i = 0; i < dlci.bindingCount; ++i)
		if (!equal(bindings[i], dlci.pBindings[i]))
			return false;
	return true;
}

}

PipelineLayoutCache::PipelineLayoutCache(Context* context)
	: m_context(context)
{
}

PipelineLayoutCache::~PipelineLayoutCache()
{
	for (auto& it : m_entries)
	{
		for (auto& entry : it.second)
		{
			vkDestroyDescriptorSetLayout(m_context->getLogicalDevice(), entry.descriptorSetLayout, 0);
			vkDestroyPipelineLayout(m_context->getLogicalDevice(), entry.pipelineLayout, 0);
		}
	}

	for (auto& it : m_samplers)
		vkDestroySampler(m_context->getLogicalDevice(), it.second, 0);
}

bool PipelineLayoutCache::get(uint32_t pipelineHash, bool useTargetSize, const VkDescriptorSetLayoutCreateInfo& dlci, VkDescriptorSetLayout& outDescriptorSetLayout, VkPipelineLayout& outPipelineLayout)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	// The hash is only a hint; reuse a layout only if it describes the same bindings.
	auto& entries = m_entries[pipelineHash];
	for (const auto& entry : entries)
	{
		if (entry.useTargetSize == useTargetSize && equal(entry.bindings, dlci))
		{
			outDescriptorSetLayout = entry.descriptorSetLayout;
			outPipelineLayout = entry.pipelineLayout;
			return true;
		}
	}
	if (!entries.empty())
		log::warning << L"Pipeline layout hash collision; creating separate layout." << Endl;

	if (vkCreateDescriptorSetLayout(m_context->getLogicalDevice(), &dlci, nullptr, &outDescriptorSetLayout) != VK_SUCCESS)
		return false;

	// Order defines each layout's set index, as used by generated shaders.
	const VkDescriptorSetLayout setLayouts[] = {
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
	{
		vkDestroyDescriptorSetLayout(m_context->getLogicalDevice(), outDescriptorSetLayout, 0);
		return false;
	}

	auto& entry = entries.push_back();
	entry.bindings.insert(entry.bindings.end(), dlci.pBindings, dlci.pBindings + dlci.bindingCount);
	entry.useTargetSize = useTargetSize;
	entry.descriptorSetLayout = outDescriptorSetLayout;
	entry.pipelineLayout = outPipelineLayout;
	return true;
}

VkSampler PipelineLayoutCache::getSampler(const VkSamplerCreateInfo& sci)
{
	Murmur3 cs;
	cs.begin();
	cs.feedBuffer(&sci, sizeof(sci));
	cs.end();

	const uint32_t samplerHash = cs.get();
	VkSampler sampler = 0;

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		auto it = m_samplers.find(samplerHash);
		if (it != m_samplers.end())
			return it->second;

		if (vkCreateSampler(m_context->getLogicalDevice(), &sci, nullptr, &sampler) != VK_SUCCESS)
			return 0;

		m_context->setObjectDebugName(L"Sampler", (uint64_t)sampler, VK_OBJECT_TYPE_SAMPLER);

		m_samplers.insert(samplerHash, sampler);
	}

	return sampler;
}

}
