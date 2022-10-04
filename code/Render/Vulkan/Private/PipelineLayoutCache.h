#pragma once

#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"
#include "Render/Vulkan/Private/ApiHeader.h"

namespace traktor::render
{
	
class PipelineLayoutCache : public Object
{
public:
	explicit PipelineLayoutCache(VkDevice logicalDevice);

	virtual ~PipelineLayoutCache();

	bool get(uint32_t pipelineHash, const VkDescriptorSetLayoutCreateInfo& dlci, VkDescriptorSetLayout& outDescriptorSetLayout, VkPipelineLayout& outPipelineLayout);

	VkSampler getSampler(const VkSamplerCreateInfo& sci);

private:
	struct Entry
	{
		VkDescriptorSetLayout descriptorSetLayout;
		VkPipelineLayout pipelineLayout;
#if defined(_DEBUG)
		std::wstring debug;
#endif
	};

	VkDevice m_logicalDevice = 0;;
	SmallMap< uint32_t, Entry > m_entries;
	SmallMap< uint32_t, VkSampler > m_samplers;
};

}