#pragma once

#include "Core/Config.h"
#include "Render/Vulkan/ApiHeader.h"

namespace traktor
{
	namespace render
	{

// Mappings between our enums and Vulkan.
extern const VkCullModeFlagBits c_cullMode[];
extern const VkCompareOp c_compareOperations[];
extern const VkStencilOp c_stencilOperations[];
extern const VkBlendFactor c_blendFactors[];
extern const VkBlendOp c_blendOperations[];
extern const VkPrimitiveTopology c_primitiveTopology[];
extern const VkFilter c_filters[];
extern const VkSamplerMipmapMode c_mipMapModes[];
extern const VkSamplerAddressMode c_addressModes[];
extern const VkFormat c_vkTextureFormats[];
extern const VkFormat c_vkTextureFormats_sRGB[];
extern const VkFormat c_vkVertexElementFormats[];

uint32_t getMemoryTypeIndex(VkPhysicalDevice physicalDevice, VkMemoryPropertyFlags memoryFlags, const VkMemoryRequirements& memoryRequirements);

bool createBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryFlags, VkBuffer& outBuffer, VkDeviceMemory& outBufferMemory);

VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);

void endSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkQueue queue);

bool changeImageLayout(
	VkDevice device,
	VkCommandPool commandPool,
	VkQueue queue,
	VkImage image,
	VkImageLayout oldLayout,
	VkImageLayout newLayout,
	int32_t mipLevel,
	int32_t mipCount,
	int32_t layer,
	int32_t layerCount,
	uint32_t aspect
);

const wchar_t* getHumanResult(VkResult result);

	}
}

