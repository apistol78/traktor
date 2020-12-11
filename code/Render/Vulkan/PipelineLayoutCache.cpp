#include "Core/Misc/Adler32.h"
#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/PipelineLayoutCache.h"

namespace traktor
{
	namespace render
	{

PipelineLayoutCache::PipelineLayoutCache(VkDevice logicalDevice)
:	m_logicalDevice(logicalDevice)
{
}

PipelineLayoutCache::~PipelineLayoutCache()
{
	for (auto& it : m_pipelineLayouts)
	{
		vkDestroyDescriptorSetLayout(m_logicalDevice, it.second.first, 0);
		vkDestroyPipelineLayout(m_logicalDevice, it.second.second, 0);
	}
}

bool PipelineLayoutCache::get(const VkDescriptorSetLayoutCreateInfo& dlci, VkDescriptorSetLayout& outDescriptorSetLayout, VkPipelineLayout& outPipelineLayout)
{
	Adler32 checksum;
	checksum.begin();
	for (uint32_t i = 0; i < dlci.bindingCount; ++i)
		checksum.feed(dlci.pBindings[i]);
	checksum.end();

	auto it = m_pipelineLayouts.find(checksum.get());
	if (it != m_pipelineLayouts.end())
	{
		outDescriptorSetLayout = it->second.first;
		outPipelineLayout = it->second.second;
		return true;
	}
	else
	{
		if (vkCreateDescriptorSetLayout(m_logicalDevice, &dlci, nullptr, &outDescriptorSetLayout) != VK_SUCCESS)
			return false;

		VkPipelineLayoutCreateInfo lci = {};
		lci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		lci.setLayoutCount = 1;
		lci.pSetLayouts = &outDescriptorSetLayout;
		lci.pushConstantRangeCount = 0;
		lci.pPushConstantRanges = nullptr;
		if (vkCreatePipelineLayout(m_logicalDevice, &lci, nullptr, &outPipelineLayout) != VK_SUCCESS)
			return false;

		m_pipelineLayouts[checksum.get()] = std::make_pair(
			outDescriptorSetLayout,
			outPipelineLayout
		);
		return true;
	}
}

	}
}
