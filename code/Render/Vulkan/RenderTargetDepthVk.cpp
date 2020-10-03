#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Render/Types.h"
#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/CommandBufferPool.h"
#include "Render/Vulkan/Queue.h"
#include "Render/Vulkan/RenderTargetDepthVk.h"
#include "Render/Vulkan/UtilitiesVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetDepthVk", RenderTargetDepthVk, ISimpleTexture)

RenderTargetDepthVk::RenderTargetDepthVk(
	VkPhysicalDevice physicalDevice,
	VkDevice logicalDevice,
	VmaAllocator allocator,
	Queue* graphicsQueue,
	CommandBufferPool* graphicsCommandPool
)
:	m_physicalDevice(physicalDevice)
,	m_logicalDevice(logicalDevice)
,	m_allocator(allocator)
,	m_graphicsQueue(graphicsQueue)
,	m_graphicsCommandPool(graphicsCommandPool)
,	m_format(VK_FORMAT_UNDEFINED)
,	m_image(0)
,	m_allocation(0)
,	m_imageView(0)
,	m_imageLayout(VK_IMAGE_LAYOUT_UNDEFINED)
,	m_haveStencil(false)
,	m_width(0)
,	m_height(0)
{
}

RenderTargetDepthVk::~RenderTargetDepthVk()
{
	destroy();
}

bool RenderTargetDepthVk::createPrimary(int32_t width, int32_t height, VkFormat format, VkImage image, const wchar_t* const tag)
{
	VkImageViewCreateInfo ivci = {};
	ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ivci.image = image;
	ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
	ivci.format = format;
	ivci.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
	ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	ivci.subresourceRange.baseMipLevel = 0;
	ivci.subresourceRange.levelCount = 1;
	ivci.subresourceRange.baseArrayLayer = 0;
	ivci.subresourceRange.layerCount = 1;

 	if (vkCreateImageView(m_logicalDevice, &ivci, nullptr, &m_imageView) != VK_SUCCESS)
		return false;

	m_format = format;
	m_image = image;
	m_haveStencil = true;
	m_width = width;
	m_height = height;

	// Set debug name of texture.
	setObjectDebugName(m_logicalDevice, tag, (uint64_t)m_image, VK_OBJECT_TYPE_IMAGE);

	// Prepare for target.
	VkCommandBuffer commandBuffer = m_graphicsCommandPool->acquireAndBegin();

	prepareAsTarget(commandBuffer);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo si = {};
	si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	si.commandBufferCount = 1;
	si.pCommandBuffers = &commandBuffer;
	m_graphicsQueue->submitAndWait(si);

	m_graphicsCommandPool->release(commandBuffer);
	return true;
}

bool RenderTargetDepthVk::create(const RenderTargetSetCreateDesc& setDesc, const wchar_t* const tag)
{
	VkResult result;

	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;

	if (setDesc.ignoreStencil)
		imageCreateInfo.format = VK_FORMAT_D32_SFLOAT;
	else
	{
#if defined(__IOS__)
		imageCreateInfo.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
#else
		imageCreateInfo.format = VK_FORMAT_D24_UNORM_S8_UINT;
#endif
	}

	imageCreateInfo.extent.width = setDesc.width;
	imageCreateInfo.extent.height = setDesc.height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

	imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	if (setDesc.usingDepthStencilAsTexture)
		imageCreateInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;

	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.queueFamilyIndexCount = 0;
	imageCreateInfo.pQueueFamilyIndices = nullptr;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	
	VmaAllocationCreateInfo aci = {};
	aci.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	if (vmaCreateImage(m_allocator, &imageCreateInfo, &aci, &m_image, &m_allocation, nullptr) != VK_SUCCESS)
		return false;	

	VkImageViewCreateInfo ivci = {};
	ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ivci.image = m_image;
	ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
	ivci.format = imageCreateInfo.format;
	ivci.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };

	if (setDesc.ignoreStencil)
		ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	else
		ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

	ivci.subresourceRange.baseMipLevel = 0;
	ivci.subresourceRange.levelCount = 1;
	ivci.subresourceRange.baseArrayLayer = 0;
	ivci.subresourceRange.layerCount = 1;
 	if ((result = vkCreateImageView(m_logicalDevice, &ivci, nullptr, &m_imageView)) != VK_SUCCESS)
	{
		log::error << L"RenderTargetDepthVk::create failed; vkCreateImageView returned error " << getHumanResult(result) << L"." << Endl;
		return false;
	}

	m_format = imageCreateInfo.format;
	m_haveStencil = !setDesc.ignoreStencil;
	m_width = setDesc.width;
	m_height = setDesc.height;

	// Set debug name of texture.
	setObjectDebugName(m_logicalDevice, tag, (uint64_t)m_image, VK_OBJECT_TYPE_IMAGE);

	// Prepare for target.
	VkCommandBuffer commandBuffer = m_graphicsCommandPool->acquireAndBegin();

	prepareAsTarget(commandBuffer);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo si = {};
	si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	si.commandBufferCount = 1;
	si.pCommandBuffers = &commandBuffer;
	m_graphicsQueue->submitAndWait(si);

	m_graphicsCommandPool->release(commandBuffer);
	return true;
}

void RenderTargetDepthVk::destroy()
{
	// Do not destroy image unless we have allocated memory for it;
	// otherwise it's primary targets thus owned by swapchain.
	if (m_allocation == 0)
		return;

	vmaFreeMemory(m_allocator, m_allocation);
	m_allocation = 0;	

	if (m_image != 0)
	{
		vkDestroyImage(m_logicalDevice, m_image, 0);
		m_image = 0;
	}
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

void RenderTargetDepthVk::prepareAsTarget(VkCommandBuffer cmdBuffer)
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

	imb.srcAccessMask = getAccessMask(imb.oldLayout);
	imb.dstAccessMask = getAccessMask(imb.newLayout);

	vkCmdPipelineBarrier(
		cmdBuffer,
		getPipelineStageFlags(imb.oldLayout),
		getPipelineStageFlags(imb.newLayout),
		0,
		0, nullptr,
		0, nullptr,
		1, &imb
	);

	m_imageLayout = imb.newLayout;
}

void RenderTargetDepthVk::prepareAsTexture(VkCommandBuffer cmdBuffer)
{
	if (m_imageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		return;

	T_ASSERT_M(m_imageLayout != VK_IMAGE_LAYOUT_UNDEFINED, L"RT have not been rendered into yet.");

	VkImageMemoryBarrier imb = {};
	imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imb.oldLayout = m_imageLayout;
	imb.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.image = m_image;

	if (m_haveStencil)
		imb.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 };
	else
		imb.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };

	imb.srcAccessMask = getAccessMask(imb.oldLayout);
	imb.dstAccessMask = getAccessMask(imb.newLayout);

	vkCmdPipelineBarrier(
		cmdBuffer,
		getPipelineStageFlags(imb.oldLayout),
		getPipelineStageFlags(imb.newLayout),
		0,
		0, nullptr,
		0, nullptr,
		1, &imb
	);

	m_imageLayout = imb.newLayout;
}

	}
}
