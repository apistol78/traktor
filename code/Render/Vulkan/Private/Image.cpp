#include "Core/Config.h"
#include "Core/Log/Log.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/Image.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/Utilities.h"

namespace traktor
{
    namespace render
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.Image", Image, Object)

Image::Image(Context* context)
:	m_context(context)
{
}

Image::~Image()
{
	T_FATAL_ASSERT_M(m_context == nullptr, L"Not properly destroyed.");
}

bool Image::createSimple(
	uint32_t width,
	uint32_t height,
	uint32_t mipLevels,
	VkFormat format,
	uint32_t usageBits
)
{
	T_FATAL_ASSERT(m_image == 0);

	// Create image.
	VkImageCreateInfo ici = {};
	ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ici.imageType = VK_IMAGE_TYPE_2D;
	ici.extent.width = width;
	ici.extent.height = height;
	ici.extent.depth = 1;
	ici.mipLevels = mipLevels;
	ici.arrayLayers = 1;
	ici.format = format;
	ici.tiling = VK_IMAGE_TILING_OPTIMAL;
	ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	ici.usage = usageBits;
	ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ici.samples = VK_SAMPLE_COUNT_1_BIT;
	ici.flags = 0;

	VmaAllocationCreateInfo aci = {};
	aci.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	if (vmaCreateImage(m_context->getAllocator(), &ici, &aci, &m_image, &m_allocation, nullptr) != VK_SUCCESS)
	{
		log::error << L"Failed to create image; unable to allocate image memory." << Endl;
		return false;
	}

	setObjectDebugName(m_context->getLogicalDevice(), T_FILE_LINE_W, (uint64_t)m_image, VK_OBJECT_TYPE_IMAGE);

	// Create image view.
	VkImageViewCreateInfo ivci = {};
	ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ivci.image = m_image;
	ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
	ivci.format = format;
	ivci.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	ivci.subresourceRange.baseMipLevel = 0;
	ivci.subresourceRange.levelCount = mipLevels;
	ivci.subresourceRange.baseArrayLayer = 0;
	ivci.subresourceRange.layerCount = 1;
	if (vkCreateImageView(m_context->getLogicalDevice(), &ivci, nullptr, &m_imageView) != VK_SUCCESS)
	{
		log::error << L"Failed to create image view; unable to create image view." << Endl;
		return false;
	}

    return true;
}

bool Image::createCube(
	uint32_t width,
	uint32_t height,
	uint32_t mipLevels,
	VkFormat format,
	uint32_t usageBits
)
{
	T_FATAL_ASSERT(m_image == 0);

	// Create image.
	VkImageCreateInfo ici = {};
	ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ici.imageType = VK_IMAGE_TYPE_2D;
	ici.extent.width = width;
	ici.extent.height = height;
	ici.extent.depth = 1;
	ici.mipLevels = mipLevels;
	ici.arrayLayers = 6;
	ici.format = format;
	ici.tiling = VK_IMAGE_TILING_OPTIMAL;
	ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	ici.usage = usageBits;
	ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ici.samples = VK_SAMPLE_COUNT_1_BIT;
	ici.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

	VmaAllocationCreateInfo aci = {};
	aci.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	if (vmaCreateImage(m_context->getAllocator(), &ici, &aci, &m_image, &m_allocation, nullptr) != VK_SUCCESS)
	{
		log::error << L"Failed to create image; unable to allocate image memory." << Endl;
		return false;
	}

	setObjectDebugName(m_context->getLogicalDevice(), T_FILE_LINE_W, (uint64_t)m_image, VK_OBJECT_TYPE_IMAGE);

	// Create image view.
	VkImageViewCreateInfo ivci = {};
	ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ivci.image = m_image;
	ivci.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	ivci.format = format;
	ivci.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	ivci.subresourceRange.baseMipLevel = 0;
	ivci.subresourceRange.levelCount = mipLevels;
	ivci.subresourceRange.baseArrayLayer = 0;
	ivci.subresourceRange.layerCount = 6;
	if (vkCreateImageView(m_context->getLogicalDevice(), &ivci, nullptr, &m_imageView) != VK_SUCCESS)
	{
		log::error << L"Failed to create image view; unable to create image view." << Endl;
		return false;
	}

    return true;
}

void Image::destroy()
{
	T_FATAL_ASSERT(m_locked == nullptr);
	if (m_allocation != 0)
	{
		m_context->addDeferredCleanup([
			image = m_image,
			allocation = m_allocation
		](Context* cx) {
			vmaDestroyImage(cx->getAllocator(), image, allocation);
		});
	}
	if (m_imageView != 0)
	{
		m_context->addDeferredCleanup([
			imageView = m_imageView
		](Context* cx) {
			vkDestroyImageView(cx->getLogicalDevice(), imageView, nullptr);
		});
	}
	m_allocation = 0;
	m_image = 0;
	m_imageView = 0;
	m_context = nullptr;
}

void* Image::lock()
{
	if (m_locked)
		return m_locked;

	if (vmaMapMemory(m_context->getAllocator(), m_allocation, &m_locked) != VK_SUCCESS)
		m_locked = nullptr;

	return m_locked;
}

void Image::unlock()
{
	if (m_locked)
	{
		vmaUnmapMemory(m_context->getAllocator(), m_allocation);
		m_locked = nullptr;
	}
}

    }
}