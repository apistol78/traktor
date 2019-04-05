#include <cstring>
#include "Render/Types.h"
#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/SimpleTextureVk.h"
#include "Render/Vulkan/UtilitiesVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SimpleTextureVk", SimpleTextureVk, ISimpleTexture)

SimpleTextureVk::SimpleTextureVk()
:	m_textureImage(nullptr)
,	m_textureView(nullptr)
,	m_width(0)
,	m_height(0)
{
}

SimpleTextureVk::~SimpleTextureVk()
{
	destroy();
}

bool SimpleTextureVk::create(
	VkPhysicalDevice physicalDevice,
	VkDevice device,
	VkCommandPool commandPool,
	VkQueue queue,
	const SimpleTextureCreateDesc& desc
)
{
	if (desc.immutable)
	{
		// Create texture image.
		VkImageCreateInfo ici = {};
		ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		ici.imageType = VK_IMAGE_TYPE_2D;
		ici.extent.width = desc.width;
		ici.extent.height = desc.height;
		ici.extent.depth = 1;
		ici.mipLevels = 1; // desc.mipCount;
		ici.arrayLayers = 1;
		ici.format = VK_FORMAT_R8G8B8A8_UNORM;
		ici.tiling = VK_IMAGE_TILING_LINEAR;
		ici.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
		ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ici.samples = VK_SAMPLE_COUNT_1_BIT;
		ici.flags = 0;
		
		if (vkCreateImage(device, &ici, nullptr, &m_textureImage) != VK_SUCCESS)
			return false;

		// Calculate memory requirement of texture image.
		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(device, m_textureImage, &memoryRequirements);

		// Allocate texture memory.
		VkDeviceMemory textureImageMemory = nullptr;

		VkMemoryAllocateInfo mai = {};
		mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		mai.allocationSize = memoryRequirements.size;
		mai.memoryTypeIndex = getMemoryTypeIndex(physicalDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, memoryRequirements);

		if (vkAllocateMemory(device, &mai, nullptr, &textureImageMemory) != VK_SUCCESS)
			return false;

		vkBindImageMemory(device, m_textureImage, textureImageMemory, 0);

		// Copy source data into texture image.
		uint32_t mipSize = getTextureMipPitch(desc.format, desc.width, desc.height, 0);

		void* data = nullptr;
		if (vkMapMemory(device, textureImageMemory, 0, mipSize, 0, &data) != VK_SUCCESS)
			return false;

		std::memcpy(data, desc.initialData[0].data, mipSize);

		VkMappedMemoryRange mmr = {};
		mmr.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mmr.memory = textureImageMemory;
		mmr.offset = 0;
		mmr.size = VK_WHOLE_SIZE;
		vkFlushMappedMemoryRanges(device, 1, &mmr);

		vkUnmapMemory(device, textureImageMemory);

		// Create texture view.
		VkImageViewCreateInfo ivci = {};
		ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ivci.image = m_textureImage;
		ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ivci.format = VK_FORMAT_R8G8B8A8_UNORM;
		ivci.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ivci.subresourceRange.baseMipLevel = 0;
		ivci.subresourceRange.levelCount = 1;
		ivci.subresourceRange.baseArrayLayer = 0;
		ivci.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &ivci, NULL, &m_textureView) != VK_SUCCESS)
			return false;

		// Transition image layout into optimal GPU read.
		changeImageLayout(
			device,
			commandPool,
			queue,
			m_textureImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		);
	}

	return true;
}

void SimpleTextureVk::destroy()
{
}

ITexture* SimpleTextureVk::resolve()
{
	return this;
}

int32_t SimpleTextureVk::getMips() const
{
	return 1;
}

int32_t SimpleTextureVk::getWidth() const
{
	return m_width;
}

int32_t SimpleTextureVk::getHeight() const
{
	return m_height;
}

bool SimpleTextureVk::lock(int32_t level, Lock& lock)
{
	return true;
}

void SimpleTextureVk::unlock(int32_t level)
{
}

void* SimpleTextureVk::getInternalHandle()
{
	return nullptr;
}

	}
}
