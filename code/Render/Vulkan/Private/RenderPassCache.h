#pragma once

#include "Core/Object.h"
#include "Render/Types.h"
#include "Render/Vulkan/Private/ApiHeader.h"

namespace traktor::render
{

class RenderPassCache : public Object
{
	T_RTTI_CLASS;

public:
#pragma pack(1)
	struct Specification
	{
		uint8_t msaaSampleCount;
		uint8_t clear;
		uint8_t load;
		uint8_t store;
		VkFormat colorTargetFormats[RenderTargetSetCreateDesc::MaxTargets];
		VkFormat depthTargetFormat;
	};
#pragma pack()

	explicit RenderPassCache(VkDevice logicalDevice);

	bool get(
		const Specification& spec,
		VkRenderPass& outRenderPass
	);

private:
	VkDevice m_logicalDevice;
	SmallMap< Specification, VkRenderPass > m_renderPasses;
};

}
