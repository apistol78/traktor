#include <cstring>
#include "Core/Misc/TString.h"
#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/UtilitiesVk.h"

namespace traktor
{
	namespace render
	{

const VkCullModeFlagBits c_cullMode[] =
{
	VK_CULL_MODE_NONE,
	VK_CULL_MODE_FRONT_BIT,
	VK_CULL_MODE_BACK_BIT
};

const VkCompareOp c_compareOperations[] =
{
	VK_COMPARE_OP_ALWAYS,
	VK_COMPARE_OP_NEVER,
	VK_COMPARE_OP_LESS,
	VK_COMPARE_OP_LESS_OR_EQUAL,
	VK_COMPARE_OP_GREATER,
	VK_COMPARE_OP_GREATER_OR_EQUAL,
	VK_COMPARE_OP_EQUAL,
	VK_COMPARE_OP_NOT_EQUAL,
	VK_COMPARE_OP_NEVER
};

const VkStencilOp c_stencilOperations[] =
{
	VK_STENCIL_OP_KEEP,
	VK_STENCIL_OP_ZERO,
	VK_STENCIL_OP_REPLACE,
	VK_STENCIL_OP_INCREMENT_AND_CLAMP,
	VK_STENCIL_OP_DECREMENT_AND_CLAMP,
	VK_STENCIL_OP_INVERT,
	VK_STENCIL_OP_INCREMENT_AND_WRAP,
	VK_STENCIL_OP_DECREMENT_AND_WRAP
};

const VkBlendFactor c_blendFactors[] =
{
	VK_BLEND_FACTOR_ONE,
	VK_BLEND_FACTOR_ZERO,
	VK_BLEND_FACTOR_SRC_COLOR,
	VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
	VK_BLEND_FACTOR_DST_COLOR,
	VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
	VK_BLEND_FACTOR_SRC_ALPHA,
	VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
	VK_BLEND_FACTOR_DST_ALPHA,
	VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA
};

const VkBlendOp c_blendOperations[] =
{
	VK_BLEND_OP_ADD,
	VK_BLEND_OP_SUBTRACT,
	VK_BLEND_OP_REVERSE_SUBTRACT,
	VK_BLEND_OP_MIN,
	VK_BLEND_OP_MAX
};

const VkPrimitiveTopology c_primitiveTopology[] =
{
	VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
	VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
	VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
	VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
	VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
};

const VkFilter c_filters[] =
{
	VK_FILTER_NEAREST,
	VK_FILTER_LINEAR
};

const VkSamplerMipmapMode c_mipMapModes[] =
{
	VK_SAMPLER_MIPMAP_MODE_NEAREST,
	VK_SAMPLER_MIPMAP_MODE_LINEAR
};

const VkSamplerAddressMode c_addressModes[] =
{
	VK_SAMPLER_ADDRESS_MODE_REPEAT,
	VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
	VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
	VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER
};

const VkFormat c_vkTextureFormats[] =
{
	VK_FORMAT_UNDEFINED,

	VK_FORMAT_R8_UNORM,					// TfR8
	VK_FORMAT_R8G8B8A8_UNORM,			// TfR8G8B8A8
	VK_FORMAT_R5G6B5_UNORM_PACK16,		// TfR5G6B5
	VK_FORMAT_R5G5B5A1_UNORM_PACK16,	// TfR5G5B5A1
	VK_FORMAT_R4G4B4A4_UNORM_PACK16,	// TfR4G4B4A4
	VK_FORMAT_A2R10G10B10_UNORM_PACK32,	// TfR10G10B10A2

	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,

	VK_FORMAT_R16G16B16A16_SFLOAT,		// TfR16G16B16A16F
	VK_FORMAT_R32G32B32A32_SFLOAT,		// TfR32G32B32A32F
	VK_FORMAT_R16G16_SFLOAT,			// TfR16G16F
	VK_FORMAT_R32G32_SFLOAT,			// TfR32G32F
	VK_FORMAT_R16_SFLOAT,				// TfR16F
	VK_FORMAT_R32_SFLOAT,				// TfR32F

	VK_FORMAT_B10G11R11_UFLOAT_PACK32,	// TfR11G11B10F

	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,

	VK_FORMAT_BC1_RGBA_UNORM_BLOCK,		// TfDXT1
	VK_FORMAT_BC2_UNORM_BLOCK,			// TfDXT2
	VK_FORMAT_BC2_UNORM_BLOCK,			// TfDXT3
	VK_FORMAT_BC3_UNORM_BLOCK,			// TfDXT4
	VK_FORMAT_BC3_UNORM_BLOCK,			// TfDXT5

	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,

	VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG,	// TfPVRTC1
	VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG,	// TfPVRTC2
	VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG,	// TfPVRTC3
	VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG,	// TfPVRTC4
	VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,	// TfETC1

	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED
};

const VkFormat c_vkTextureFormats_sRGB[] =
{
	VK_FORMAT_UNDEFINED,

	VK_FORMAT_UNDEFINED,
	VK_FORMAT_R8G8B8A8_SRGB,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,

	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,

	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,

	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,

	VK_FORMAT_BC1_RGBA_SRGB_BLOCK,	// DXT1
	VK_FORMAT_BC2_SRGB_BLOCK,	// DXT2
	VK_FORMAT_BC3_SRGB_BLOCK,	// DXT3
	VK_FORMAT_BC4_UNORM_BLOCK,	// DXT4
	VK_FORMAT_BC5_UNORM_BLOCK,	// DXT5

	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,

	VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG,	// TfPVRTC1
	VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG,	// TfPVRTC2
	VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG,	// TfPVRTC3
	VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG,	// TfPVRTC4
	VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK,	// TfETC1

	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED
};

const VkFormat c_vkVertexElementFormats[] =
{
	VK_FORMAT_R32_SFLOAT, // DtFloat1
	VK_FORMAT_R32G32_SFLOAT, // DtFloat2
	VK_FORMAT_R32G32B32_SFLOAT, // DtFloat3
	VK_FORMAT_R32G32B32A32_SFLOAT, // DtFloat4
	VK_FORMAT_R8G8B8A8_SNORM, // DtByte4
	VK_FORMAT_R8G8B8A8_UNORM, // DtByte4N
	VK_FORMAT_R16G16_SNORM, // DtShort2
	VK_FORMAT_R16G16B16A16_SNORM, // DtShort4
	VK_FORMAT_R16G16_UNORM, // DtShort2N
	VK_FORMAT_R16G16B16A16_UNORM, // DtShort4N
	VK_FORMAT_R16G16_SFLOAT, // DtHalf2
	VK_FORMAT_R16G16B16A16_SFLOAT // DtHalf4
};

struct TargetFormat
{
	VkFormat formats[4];
};

const TargetFormat c_vkTargetFormats[] =
{
	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },

	{ { VK_FORMAT_R8_UNORM,                 VK_FORMAT_R8G8B8A8_UNORM,      VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },	// TfR8
	{ { VK_FORMAT_R8G8B8A8_UNORM,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },	// TfR8G8B8A8
	{ { VK_FORMAT_R5G6B5_UNORM_PACK16,      VK_FORMAT_R8G8B8A8_UNORM,      VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },	// TfR5G6B5
	{ { VK_FORMAT_R5G5B5A1_UNORM_PACK16,    VK_FORMAT_R8G8B8A8_UNORM,      VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },	// TfR5G5B5A1
	{ { VK_FORMAT_R4G4B4A4_UNORM_PACK16,    VK_FORMAT_R8G8B8A8_UNORM,      VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },	// TfR4G4B4A4
	{ { VK_FORMAT_A2R10G10B10_UNORM_PACK32, VK_FORMAT_R8G8B8A8_UNORM,      VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },	// TfR10G10B10A2

	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },
	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },
	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },

	{ { VK_FORMAT_R16G16B16A16_SFLOAT,      VK_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },	// TfR16G16B16A16F
	{ { VK_FORMAT_R32G32B32A32_SFLOAT,      VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },	// TfR32G32B32A32F
	{ { VK_FORMAT_R16G16_SFLOAT,            VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_R32G32_SFLOAT,       VK_FORMAT_UNDEFINED } },	// TfR16G16F
	{ { VK_FORMAT_R32G32_SFLOAT,            VK_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_R16G16_SFLOAT,       VK_FORMAT_UNDEFINED } },	// TfR32G32F
	{ { VK_FORMAT_R16_SFLOAT,               VK_FORMAT_R32_SFLOAT,          VK_FORMAT_R16G16_SFLOAT,       VK_FORMAT_R32G32_SFLOAT } },	// TfR16F
	{ { VK_FORMAT_R32_SFLOAT,               VK_FORMAT_R16_SFLOAT,          VK_FORMAT_R32G32_SFLOAT,       VK_FORMAT_R16G16_SFLOAT } },	// TfR32F

	{ { VK_FORMAT_B10G11R11_UFLOAT_PACK32,  VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_UNDEFINED } },	// TfR11G11B10F

	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },
	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },
	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },
	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },
	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },
	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },
	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },
	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },
	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },
	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },
	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },
	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },
	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },

	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },	// TfDXT1
	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },	// TfDXT2
	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },	// TfDXT3
	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },	// TfDXT4
	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },	// TfDXT5

	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },
	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },
	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },
	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },
	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },

	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },	// TfPVRTC1
	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },	// TfPVRTC2
	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },	// TfPVRTC3
	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },	// TfPVRTC4
	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },	// TfETC1

	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },
	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },
	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } },
	{ { VK_FORMAT_UNDEFINED,                VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED,           VK_FORMAT_UNDEFINED } }
};

uint32_t getMemoryTypeIndex(VkPhysicalDevice physicalDevice, VkMemoryPropertyFlags memoryFlags, const VkMemoryRequirements& memoryRequirements)
{
	VkPhysicalDeviceMemoryProperties memoryProperties = {};
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

	uint32_t memoryTypeBits = memoryRequirements.memoryTypeBits;
	for (uint32_t i = 0; i < 32; ++i)
	{
		VkMemoryType memoryType = memoryProperties.memoryTypes[i];
		if (memoryTypeBits & 1)
		{
			if ((memoryType.propertyFlags & memoryFlags) == memoryFlags)
				return i;
		}
		memoryTypeBits = memoryTypeBits >> 1;
	}
	return 0;
}

bool createBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryFlags, VkBuffer& outBuffer, VkDeviceMemory& outBufferMemory)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &bufferInfo, nullptr, &outBuffer) != VK_SUCCESS)
		return false;

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, outBuffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = getMemoryTypeIndex(physicalDevice, memoryFlags, memRequirements);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &outBufferMemory) != VK_SUCCESS)
		return false;

	vkBindBufferMemory(device, outBuffer, outBufferMemory, 0);
	return true;
}

VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool)
{
	// Allocate command buffer.
	VkCommandBuffer commandBuffer = nullptr;

	VkCommandBufferAllocateInfo cbai = {};
	cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cbai.commandPool = commandPool;
	cbai.commandBufferCount = 1;

	vkAllocateCommandBuffers(device, &cbai, &commandBuffer);

	// Begin recording command buffer.
	VkCommandBufferBeginInfo cbbi = {};
	cbbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cbbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &cbbi);

	return commandBuffer;
}

void endSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkQueue queue)
{
	// End recording command buffer.
	vkEndCommandBuffer(commandBuffer);

	// Submit command buffer onto queue and wait until command has been executed.
	VkSubmitInfo si = {};
	si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	si.commandBufferCount = 1;
	si.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(queue, 1, &si, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

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
)
{
	auto commandBuffer = beginSingleTimeCommands(device, commandPool);

	VkImageMemoryBarrier imb = {};
	imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imb.oldLayout = oldLayout;
	imb.newLayout = newLayout;
	imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.image = image;
	imb.subresourceRange.aspectMask = aspect; //VK_IMAGE_ASPECT_COLOR_BIT;
	imb.subresourceRange.baseMipLevel = mipLevel;
	imb.subresourceRange.levelCount = mipCount;
	imb.subresourceRange.baseArrayLayer = layer;
	imb.subresourceRange.layerCount = layerCount;
	imb.srcAccessMask = 0;
	imb.dstAccessMask = 0;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &imb
	);

	endSingleTimeCommands(device, commandPool, commandBuffer, queue);
	return true;
}

const wchar_t* getHumanResult(VkResult result)
{
	switch (result)
	{
	case VK_SUCCESS:
		return L"VK_SUCCESS";
	case VK_NOT_READY:
		return L"VK_NOT_READY";
	case VK_TIMEOUT:
		return L"VK_TIMEOUT";
	case VK_EVENT_SET:
		return L"VK_EVENT_SET";
	case VK_EVENT_RESET:
		return L"VK_EVENT_RESET";
	case VK_INCOMPLETE:
		return L"VK_INCOMPLETE";
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		return L"VK_ERROR_OUT_OF_HOST_MEMORY";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		return L"VK_ERROR_OUT_OF_DEVICE_MEMORY";
	case VK_ERROR_INITIALIZATION_FAILED:
		return L"VK_ERROR_INITIALIZATION_FAILED";
	case VK_ERROR_DEVICE_LOST:
		return L"VK_ERROR_DEVICE_LOST";
	case VK_ERROR_MEMORY_MAP_FAILED:
		return L"VK_ERROR_MEMORY_MAP_FAILED";
	case VK_ERROR_LAYER_NOT_PRESENT:
		return L"VK_ERROR_LAYER_NOT_PRESENT";
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		return L"VK_ERROR_EXTENSION_NOT_PRESENT";
	case VK_ERROR_FEATURE_NOT_PRESENT:
		return L"VK_ERROR_FEATURE_NOT_PRESENT";
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		return L"VK_ERROR_INCOMPATIBLE_DRIVER";
	case VK_ERROR_TOO_MANY_OBJECTS:
		return L"VK_ERROR_TOO_MANY_OBJECTS";
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		return L"VK_ERROR_FORMAT_NOT_SUPPORTED";
	case VK_ERROR_FRAGMENTED_POOL:
		return L"VK_ERROR_FRAGMENTED_POOL";
	case VK_ERROR_SURFACE_LOST_KHR:
		return L"VK_ERROR_SURFACE_LOST_KHR";
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		return L"VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
	case VK_SUBOPTIMAL_KHR:
		return L"VK_SUBOPTIMAL_KHR";
	case VK_ERROR_OUT_OF_DATE_KHR:
		return L"VK_ERROR_OUT_OF_DATE_KHR";
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
		return L"VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
	case VK_ERROR_VALIDATION_FAILED_EXT:
		return L"VK_ERROR_VALIDATION_FAILED_EXT";
	case VK_ERROR_INVALID_SHADER_NV:
		return L"VK_ERROR_INVALID_SHADER_NV";
	default:
		return L"** Unknown **";
	}
}

void setObjectDebugName(VkDevice device, const wchar_t* const tag, uint64_t object, VkObjectType objectType)
{
	// Set debug name of texture.
#if !defined(__ANDROID__) && !defined(__APPLE__)
	VkDebugUtilsObjectNameInfoEXT ni = {};
	ni.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	ni.objectType = objectType;
	ni.objectHandle = object;
	ni.pObjectName = tag ? strdup(wstombs(tag).c_str()) : "Unnamed";
	vkSetDebugUtilsObjectNameEXT(device, &ni);
#endif
}

VkFormat determineSupportedTargetFormat(VkPhysicalDevice physicalDevice, TextureFormat textureFormat)
{
	const auto& fc = c_vkTargetFormats[textureFormat];
	for (int32_t i = 0; i < 4; ++i)
	{
		if (fc.formats[i] == VK_FORMAT_UNDEFINED)
			break;

		VkFormatProperties fp = {};
		vkGetPhysicalDeviceFormatProperties(physicalDevice, fc.formats[i], &fp);
		if ((fp.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) == 0)
			continue;
		if ((fp.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) == 0)
			continue;
			
		return fc.formats[i];
	}
	return VK_FORMAT_UNDEFINED;
}

	}
}
