#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Render/Types.h"
#include "Render/Vulkan/RenderTargetDepthVk.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/Utilities.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetDepthVk", RenderTargetDepthVk, ISimpleTexture)

RenderTargetDepthVk::RenderTargetDepthVk(Context* context)
:	m_context(context)
{
}

RenderTargetDepthVk::~RenderTargetDepthVk()
{
	destroy();
}

bool RenderTargetDepthVk::createPrimary(
	int32_t width,
	int32_t height,
	VkFormat format,
	const wchar_t* const tag
)
{
	VkImageCreateInfo ici = {};
	ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ici.imageType = VK_IMAGE_TYPE_2D;
	ici.format = format;
	ici.extent = { (uint32_t)width, (uint32_t)height, 1 };
	ici.mipLevels = 1;
	ici.arrayLayers = 1;
	ici.samples = VK_SAMPLE_COUNT_1_BIT;
	ici.tiling = VK_IMAGE_TILING_OPTIMAL;
	ici.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ici.queueFamilyIndexCount = 0;
	ici.pQueueFamilyIndices = nullptr;
	ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	ici.flags = 0;

	VmaAllocationCreateInfo aci = {};
	aci.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	if (vmaCreateImage(m_context->getAllocator(), &ici, &aci, &m_image, &m_allocation, nullptr) != VK_SUCCESS)
		return false;

	VkImageViewCreateInfo ivci = {};
	ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ivci.image = m_image;
	ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
	ivci.format = format;
	ivci.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
	ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	ivci.subresourceRange.baseMipLevel = 0;
	ivci.subresourceRange.levelCount = 1;
	ivci.subresourceRange.baseArrayLayer = 0;
	ivci.subresourceRange.layerCount = 1;
 	if (vkCreateImageView(m_context->getLogicalDevice(), &ivci, nullptr, &m_imageView) != VK_SUCCESS)
		return false;

	m_format = format;
	m_haveStencil = true;
	m_width = width;
	m_height = height;

	// Set debug name of texture.
	setObjectDebugName(m_context->getLogicalDevice(), tag, (uint64_t)m_image, VK_OBJECT_TYPE_IMAGE);
	return true;
}

bool RenderTargetDepthVk::create(const RenderTargetSetCreateDesc& setDesc, const wchar_t* const tag)
{
	VkResult result;

	VkImageCreateInfo ici = {};
	ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ici.imageType = VK_IMAGE_TYPE_2D;
	if (setDesc.ignoreStencil)
#if defined(__IOS__)
		ici.format = VK_FORMAT_D16_UNORM;
#else
		ici.format = VK_FORMAT_D32_SFLOAT;
#endif
	else
#if defined(__IOS__)
		ici.format = VK_FORMAT_D16_UNORM_S8_UINT;
#else
		ici.format = VK_FORMAT_D24_UNORM_S8_UINT;
#endif
	ici.extent.width = setDesc.width;
	ici.extent.height = setDesc.height;
	ici.extent.depth = 1;
	ici.mipLevels = 1;
	ici.arrayLayers = 1;
	ici.samples = VK_SAMPLE_COUNT_1_BIT;
	ici.tiling = VK_IMAGE_TILING_OPTIMAL;
	ici.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	if (setDesc.usingDepthStencilAsTexture)
		ici.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
	ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ici.queueFamilyIndexCount = 0;
	ici.pQueueFamilyIndices = nullptr;
	ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	
	VmaAllocationCreateInfo aci = {};
	aci.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	if (vmaCreateImage(m_context->getAllocator(), &ici, &aci, &m_image, &m_allocation, nullptr) != VK_SUCCESS)
		return false;	

	VkImageViewCreateInfo ivci = {};
	ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ivci.image = m_image;
	ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
	ivci.format = ici.format;
	ivci.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
	if (setDesc.ignoreStencil)
		ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	else
		ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	ivci.subresourceRange.baseMipLevel = 0;
	ivci.subresourceRange.levelCount = 1;
	ivci.subresourceRange.baseArrayLayer = 0;
	ivci.subresourceRange.layerCount = 1;
 	if ((result = vkCreateImageView(m_context->getLogicalDevice(), &ivci, nullptr, &m_imageView)) != VK_SUCCESS)
	{
		log::error << L"RenderTargetDepthVk::create failed; vkCreateImageView returned error " << getHumanResult(result) << L"." << Endl;
		return false;
	}

	m_format = ici.format;
	m_haveStencil = !setDesc.ignoreStencil;
	m_width = setDesc.width;
	m_height = setDesc.height;

	// Set debug name of texture.
	setObjectDebugName(m_context->getLogicalDevice(), tag, (uint64_t)m_image, VK_OBJECT_TYPE_IMAGE);
	return true;
}

void RenderTargetDepthVk::destroy()
{
	if (m_context)
	{
		auto allocation = m_allocation;
		auto imageView = m_imageView;
		auto image = m_image;

		m_context->addDeferredCleanup([=](Context* cx) {
			if (imageView != 0)
				vkDestroyImageView(cx->getLogicalDevice(), imageView, nullptr);
			if (image != 0)
				vkDestroyImage(cx->getLogicalDevice(), image, 0);
			vmaFreeMemory(cx->getAllocator(), allocation);
		});
	}

	m_context = nullptr;
	m_image = 0;
	m_imageView = 0;
	m_allocation = 0;	
}

ITexture* RenderTargetDepthVk::resolve()
{
	return this;
}

int32_t RenderTargetDepthVk::getMips() const
{
	return 1;
}

int32_t RenderTargetDepthVk::getWidth() const
{
	return m_width;
}

int32_t RenderTargetDepthVk::getHeight() const
{
	return m_height;
}

bool RenderTargetDepthVk::lock(int32_t level, Lock& lock)
{
	return false;
}

void RenderTargetDepthVk::unlock(int32_t level)
{
}

void* RenderTargetDepthVk::getInternalHandle()
{
	return nullptr;
}

void RenderTargetDepthVk::prepareAsTarget(CommandBuffer* commandBuffer)
{
	if (m_imageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		return;

	VkImageMemoryBarrier imb = {};
	imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imb.oldLayout = m_imageLayout;
	imb.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.image = m_image;

	if (m_haveStencil)
		imb.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 };
	else
		imb.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };

	imb.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	imb.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	vkCmdPipelineBarrier(
		*commandBuffer,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &imb
	);

	m_imageLayout = imb.newLayout;
}

void RenderTargetDepthVk::prepareAsTexture(CommandBuffer* commandBuffer)
{
	if (m_imageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
		return;

	T_ASSERT_M(m_imageLayout != VK_IMAGE_LAYOUT_UNDEFINED, L"RT have not been rendered into yet.");

	VkImageMemoryBarrier imb = {};
	imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imb.oldLayout = m_imageLayout;
	imb.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.image = m_image;

	if (m_haveStencil)
		imb.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 };
	else
		imb.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };

	imb.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	imb.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(
		*commandBuffer,
		VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &imb
	);

	m_imageLayout = imb.newLayout;
}

	}
}
