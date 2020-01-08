#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Render/Types.h"
#include "Render/Vulkan/ApiLoader.h"
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
	VkCommandPool setupCommandPool,
	VkQueue setupQueue
)
:	m_physicalDevice(physicalDevice)
,	m_logicalDevice(logicalDevice)
,	m_allocator(allocator)
,	m_setupCommandPool(setupCommandPool)
,	m_setupQueue(setupQueue)
,	m_format(VK_FORMAT_UNDEFINED)
,	m_image(0)
,	m_allocation(0)
,	m_imageView(0)
,	m_imageLayout(VK_IMAGE_LAYOUT_UNDEFINED)
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
	m_width = width;
	m_height = height;

	// Set debug name of texture.
	setObjectDebugName(m_logicalDevice, tag, (uint64_t)m_image, VK_OBJECT_TYPE_IMAGE);

	// Prepare for target.
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(
		m_logicalDevice,
		m_setupCommandPool
	);

	prepareAsTarget(commandBuffer);

	endSingleTimeCommands(
		m_logicalDevice,
		m_setupCommandPool,
		commandBuffer,
		m_setupQueue
	);

	return true;
}

bool RenderTargetDepthVk::create(const RenderTargetSetCreateDesc& setDesc, const wchar_t* const tag)
{
	VkResult result;

	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
#if !defined(__ANDROID__)
	imageCreateInfo.format = VK_FORMAT_D32_SFLOAT;
#else
	imageCreateInfo.format = VK_FORMAT_D16_UNORM;
#endif
	imageCreateInfo.extent.width = setDesc.width;
	imageCreateInfo.extent.height = setDesc.height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	if (setDesc.usingDepthStencilAsTexture)
		imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	else
		imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
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
	ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT; // | VK_IMAGE_ASPECT_STENCIL_BIT;
	ivci.subresourceRange.baseMipLevel = 0;
	ivci.subresourceRange.levelCount = 1;
	ivci.subresourceRange.baseArrayLayer = 0;
	ivci.subresourceRange.layerCount = 1;
 	if ((result = vkCreateImageView(m_logicalDevice, &ivci, nullptr, &m_imageView)) != VK_SUCCESS)
	{
		log::error << L"RenderTargetDepthVk::create failed; vkCreateImageView returned error " << getHumanResult(result) << L"." << Endl;
		return false;
	}

	if (setDesc.usingDepthStencilAsTexture)
	{
		// Initially clear render target to a known state.
		changeImageLayout(
			m_logicalDevice,
			m_setupCommandPool,
			m_setupQueue,
			m_image,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			0,
			1,
			0,
			1,
			VK_IMAGE_ASPECT_DEPTH_BIT
		);

		VkCommandBuffer commandBuffer = beginSingleTimeCommands(
			m_logicalDevice,
			m_setupCommandPool
		);

		VkClearDepthStencilValue cdsv = { 0 };
		cdsv.depth = 0.0f;
		cdsv.stencil = 0;

		VkImageSubresourceRange isr = {};
		isr.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		isr.baseMipLevel = 0;
		isr.levelCount = 1;
		isr.baseArrayLayer = 0;
		isr.layerCount = 1;

		vkCmdClearDepthStencilImage(
			commandBuffer,
			m_image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			&cdsv,
			1,
			&isr
		);

		endSingleTimeCommands(
			m_logicalDevice,
			m_setupCommandPool,
			commandBuffer,
			m_setupQueue
		);
	}

	m_format = imageCreateInfo.format;
	m_width = setDesc.width;
	m_height = setDesc.height;

	// Set debug name of texture.
	setObjectDebugName(m_logicalDevice, tag, (uint64_t)m_image, VK_OBJECT_TYPE_IMAGE);

	// Prepare for target.
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(
		m_logicalDevice,
		m_setupCommandPool
	);

	prepareAsTarget(commandBuffer);

	endSingleTimeCommands(
		m_logicalDevice,
		m_setupCommandPool,
		commandBuffer,
		m_setupQueue
	);

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

	VkImageMemoryBarrier layoutTransitionBarrier = {};
	layoutTransitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	layoutTransitionBarrier.srcAccessMask = 0; //VK_ACCESS_MEMORY_READ_BIT;
	layoutTransitionBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	layoutTransitionBarrier.oldLayout = m_imageLayout;
	layoutTransitionBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	layoutTransitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	layoutTransitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	layoutTransitionBarrier.image = m_image;

	switch (m_format)
	{
	case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_D32_SFLOAT:
		layoutTransitionBarrier.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };
		break;

	case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
		layoutTransitionBarrier.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 };
		break;

	default:
		T_FATAL_ERROR;
		break;
	}

	vkCmdPipelineBarrier(
		cmdBuffer,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, /// VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &layoutTransitionBarrier
	);

	m_imageLayout = layoutTransitionBarrier.newLayout;
}

void RenderTargetDepthVk::prepareAsTexture(VkCommandBuffer cmdBuffer)
{
	if (m_imageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		return;

	T_ASSERT_M(m_imageLayout != VK_IMAGE_LAYOUT_UNDEFINED, L"RT have not been rendered into yet.");

	VkImageMemoryBarrier layoutTransitionBarrier = {};
	layoutTransitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	layoutTransitionBarrier.srcAccessMask = 0; //VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	layoutTransitionBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; //VK_ACCESS_MEMORY_READ_BIT;
	layoutTransitionBarrier.oldLayout = m_imageLayout;
	layoutTransitionBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	layoutTransitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	layoutTransitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	layoutTransitionBarrier.image = m_image;

	switch (m_format)
	{
	case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_D32_SFLOAT:
		layoutTransitionBarrier.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };
		break;

	case VK_FORMAT_D24_UNORM_S8_UINT:
		layoutTransitionBarrier.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 };
		break;

	default:
		T_FATAL_ERROR;
		break;
	}

	vkCmdPipelineBarrier(
		cmdBuffer,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &layoutTransitionBarrier
	);

	m_imageLayout = layoutTransitionBarrier.newLayout;
}

	}
}
