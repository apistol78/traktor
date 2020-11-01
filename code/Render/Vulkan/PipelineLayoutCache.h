#pragma once

#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"
#include "Render/Vulkan/ApiHeader.h"

namespace traktor
{
	namespace render
	{
	
class PipelineLayoutCache : public Object
{
public:
	explicit PipelineLayoutCache(VkDevice logicalDevice);

	virtual ~PipelineLayoutCache();

	bool get(const VkDescriptorSetLayoutCreateInfo& dlci, VkDescriptorSetLayout& outDescriptorSetLayout, VkPipelineLayout& outPipelineLayout);

private:
	VkDevice m_logicalDevice;
	SmallMap< uint32_t, std::pair< VkDescriptorSetLayout, VkPipelineLayout > > m_pipelineLayouts;
};

	}
}