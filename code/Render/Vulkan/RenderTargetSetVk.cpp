#pragma optimize( "", off )

#include <cstring>
#include <limits>
#include "Core/Containers/AlignedVector.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/Types.h"
#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/RenderTargetDepthVk.h"
#include "Render/Vulkan/RenderTargetVk.h"
#include "Render/Vulkan/RenderTargetSetVk.h"
#include "Render/Vulkan/UtilitiesVk.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

uint32_t s_nextId = 1;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSetVk", RenderTargetSetVk, RenderTargetSet)

RenderTargetSetVk::RenderTargetSetVk(
	VkPhysicalDevice physicalDevice,
	VkDevice logicalDevice,
	VkCommandPool setupCommandPool,
	VkQueue setupQueue
)
:	RenderTargetSet()
,	m_physicalDevice(physicalDevice)
,	m_logicalDevice(logicalDevice)
,	m_setupCommandPool(setupCommandPool)
,	m_setupQueue(setupQueue)
{
}

RenderTargetSetVk::~RenderTargetSetVk()
{
	destroy();
}

bool RenderTargetSetVk::createPrimary(int32_t width, int32_t height, VkFormat colorFormat, VkImage colorImage, VkFormat depthFormat, VkImage depthImage)
{
	m_colorTargets.resize(1);
	m_colorTargets[0] = new RenderTargetVk();
	if (!m_colorTargets[0]->createPrimary(m_physicalDevice, m_logicalDevice, width, height, colorFormat, colorImage))
		return false;

	m_depthTarget = new RenderTargetDepthVk();
	if (!m_depthTarget->createPrimary(m_physicalDevice, m_logicalDevice, width, height, depthFormat, depthImage))
		return false;

	m_setDesc.count = 1;
	m_setDesc.width = width;
	m_setDesc.height = height;
	m_setDesc.multiSample = 0;
	m_setDesc.createDepthStencil = true;
	m_setDesc.usingDepthStencilAsTexture = false;
	m_setDesc.usingPrimaryDepthStencil = false;	// We contain primary depth thus do not share.
	m_setDesc.preferTiled = false;
	m_setDesc.ignoreStencil = false;
	m_setDesc.generateMips = false;
	m_setDesc.sharedDepthStencil = nullptr;
	m_setDesc.targets[0].format = TfR8G8B8A8;
	m_setDesc.targets[0].sRGB = false;

	return true;
}

bool RenderTargetSetVk::create(const RenderTargetSetCreateDesc& setDesc)
{
	m_colorTargets.resize(setDesc.count);
	for (int32_t i = 0; i < setDesc.count; ++i)
	{
		m_colorTargets[i] = new RenderTargetVk();
		if (!m_colorTargets[i]->create(m_physicalDevice, m_logicalDevice, setDesc, setDesc.targets[i]))
			return false;
	}

	if (setDesc.createDepthStencil)
	{
		m_depthTarget = new RenderTargetDepthVk();
		if (!m_depthTarget->create(m_physicalDevice, m_logicalDevice, setDesc))
			return false;
	}

	m_setDesc = setDesc;
	return true;
}

void RenderTargetSetVk::destroy()
{
	for (auto i = m_colorTargets.begin(); i != m_colorTargets.end(); ++i)
	{
		if (*i)
			(*i)->destroy();
	}
	m_colorTargets.resize(0);
	safeDestroy(m_depthTarget);
}

int32_t RenderTargetSetVk::getWidth() const
{
	return m_setDesc.width;
}

int32_t RenderTargetSetVk::getHeight() const
{
	return m_setDesc.height;
}

ISimpleTexture* RenderTargetSetVk::getColorTexture(int32_t index) const
{
	return index < int32_t(m_colorTargets.size()) ? m_colorTargets[index] : nullptr;
}

ISimpleTexture* RenderTargetSetVk::getDepthTexture() const
{
	return m_depthTarget;
}

void RenderTargetSetVk::discard()
{
}

bool RenderTargetSetVk::isContentValid() const
{
	return true;
}

bool RenderTargetSetVk::read(int32_t index, void* buffer) const
{
	if (index < 0 || index >= (int32_t)m_colorTargets.size())
		return false;

	// Create offscreen image copy.
	VkImage hostImage = nullptr;

	VkImageCreateInfo ici = {};
	ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ici.imageType = VK_IMAGE_TYPE_2D;
	ici.format = m_colorTargets[index]->getVkFormat(); //  VK_FORMAT_R8G8B8A8_UNORM;
	ici.extent.width = m_setDesc.width;
	ici.extent.height = m_setDesc.height;
	ici.extent.depth = 1;
	ici.arrayLayers = 1;
	ici.mipLevels = 1;
	ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	ici.samples = VK_SAMPLE_COUNT_1_BIT;
	ici.tiling = VK_IMAGE_TILING_LINEAR;
	ici.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	if (vkCreateImage(m_logicalDevice, &ici, nullptr, &hostImage) != VK_SUCCESS)
		return false;

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(m_logicalDevice, hostImage, &memoryRequirements);

	VkDeviceMemory hostImageMemory = nullptr;

	VkMemoryAllocateInfo mai = {};
	mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mai.allocationSize = memoryRequirements.size;
	mai.memoryTypeIndex = getMemoryTypeIndex(m_physicalDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memoryRequirements);

	vkAllocateMemory(m_logicalDevice, &mai, nullptr, &hostImageMemory);
	vkBindImageMemory(m_logicalDevice, hostImage, hostImageMemory, 0);

	// Transfer color target into host image.
	changeImageLayout(
		m_logicalDevice,
		m_setupCommandPool,
		m_setupQueue,
		hostImage,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	);

	VkCommandBuffer commandBuffer = beginSingleTimeCommands(m_logicalDevice, m_setupCommandPool);

	m_colorTargets[index]->prepareForReadBack(commandBuffer);

	VkImageCopy ic = {};
	ic.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	ic.srcSubresource.layerCount = 1;
	ic.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	ic.dstSubresource.layerCount = 1;
	ic.extent.width = m_setDesc.width;
	ic.extent.height = m_setDesc.height;
	ic.extent.depth = 1;

	vkCmdCopyImage(
		commandBuffer,
		m_colorTargets[index]->getVkImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		hostImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&ic
	);

	changeImageLayout(
		m_logicalDevice,
		m_setupCommandPool,
		m_setupQueue,
		hostImage,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_GENERAL
	);

	endSingleTimeCommands(m_logicalDevice, m_setupCommandPool, commandBuffer, m_setupQueue);

	// Get information about image.
	VkImageSubresource isr = {};
	isr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	VkSubresourceLayout srl = {};
	vkGetImageSubresourceLayout(m_logicalDevice, hostImage, &isr, &srl);

	// Read back data.
	uint8_t* src = nullptr;
	vkMapMemory(m_logicalDevice, hostImageMemory, 0, VK_WHOLE_SIZE, 0, (void**)&src);
	src += srl.offset;

	uint32_t fragmentSize = 4 * sizeof(float);

	uint8_t* dst = (uint8_t*)buffer;
	for (uint32_t y = 0; y < m_setDesc.height; ++y)
	{
		std::memcpy(dst, src, m_setDesc.width * fragmentSize);
		dst += m_setDesc.width * fragmentSize;
		src += srl.rowPitch;
	}

	vkUnmapMemory(m_logicalDevice, hostImageMemory);
	
	// Cleanup
	vkFreeMemory(m_logicalDevice, hostImageMemory, nullptr);
	// vkDestroyImage(m_logicalDevice, hostImage, nullptr);

	return true;
}

bool RenderTargetSetVk::prepareAsTarget(
	VkCommandBuffer commandBuffer,
	int32_t colorIndex,
	const Clear& clear,
	RenderTargetDepthVk* primaryDepthTarget,

	// Out
	uint32_t& outId,
	VkRenderPass& outRenderPass
)
{
	auto key = std::make_tuple(
		colorIndex,
		clear.mask
	);

	auto& rt = m_renderPasses[key];

	if (rt.renderPass == nullptr)
	{
		rt.id = s_nextId++;

		AlignedVector< VkAttachmentDescription > passAttachments;

		if (colorIndex >= 0)
		{
			// One color target selected.
			VkAttachmentDescription passAttachment = {};
			passAttachment.format = m_colorTargets[colorIndex]->getVkFormat();
			passAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			passAttachment.loadOp = ((clear.mask & CfColor) != 0) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;// VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			passAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			passAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			passAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			passAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			passAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			passAttachments.push_back(passAttachment);
		}
		else
		{
			// Attach all color targets for MRT.
			for (int32_t i = 0; i < m_setDesc.count; ++i)
			{
				VkAttachmentDescription passAttachment = {};
				passAttachment.format = m_colorTargets[i]->getVkFormat();
				passAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
				passAttachment.loadOp = ((clear.mask & CfColor) != 0) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;// VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				passAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				passAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				passAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				passAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				passAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				passAttachments.push_back(passAttachment);
			}
		}

		if (m_depthTarget)
		{
			VkAttachmentDescription passAttachment = {};
			passAttachment.format = m_depthTarget->getVkFormat();
			passAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			passAttachment.loadOp = ((clear.mask & CfDepth) != 0) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;// VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			passAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;// VK_ATTACHMENT_STORE_OP_DONT_CARE;
			passAttachment.stencilLoadOp = ((clear.mask & CfStencil) != 0) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;// VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			passAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;//VK_ATTACHMENT_STORE_OP_DONT_CARE;
			passAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			passAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			passAttachments.push_back(passAttachment);
		}
		else if (m_setDesc.usingPrimaryDepthStencil)
		{
			VkAttachmentDescription passAttachment = {};
			passAttachment.format = primaryDepthTarget->getVkFormat();
			passAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			passAttachment.loadOp = ((clear.mask & CfDepth) != 0) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;// VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			passAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;//VK_ATTACHMENT_STORE_OP_DONT_CARE;
			passAttachment.stencilLoadOp = ((clear.mask & CfStencil) != 0) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;// VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			passAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;//VK_ATTACHMENT_STORE_OP_DONT_CARE;
			passAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			passAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			passAttachments.push_back(passAttachment);
		}

		AlignedVector< VkAttachmentReference > colorAttachmentReferences;
		VkAttachmentReference depthAttachmentReference = {};

		if (colorIndex >= 0)
		{
			auto& colorAttachmentReference = colorAttachmentReferences.push_back();
			colorAttachmentReference.attachment = 0;
			colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			depthAttachmentReference.attachment = 1;
			depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		else
		{
			for (int i = 0; i < m_setDesc.count; ++i)
			{
				auto& colorAttachmentReference = colorAttachmentReferences.push_back();
				colorAttachmentReference.attachment = i;
				colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}

			depthAttachmentReference.attachment = m_setDesc.count;
			depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = (uint32_t)colorAttachmentReferences.size();
		subpass.pColorAttachments = colorAttachmentReferences.c_ptr();
		if (m_depthTarget || m_setDesc.usingPrimaryDepthStencil)
			subpass.pDepthStencilAttachment = &depthAttachmentReference;

		VkRenderPassCreateInfo renderPassCreateInfo = {};
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = passAttachments.size();
		renderPassCreateInfo.pAttachments = passAttachments.ptr();
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpass;
		if (vkCreateRenderPass(m_logicalDevice, &renderPassCreateInfo, nullptr, &rt.renderPass) != VK_SUCCESS)
			return false;
	}

	// (Re-)create frame buffer.
	if (rt.frameBuffer == nullptr)
	{
 		AlignedVector< VkImageView > frameBufferAttachments;
		
		if (colorIndex >= 0)
			frameBufferAttachments.push_back(m_colorTargets[colorIndex]->getVkImageView());
		else
		{
			for (int32_t i = 0; i < m_setDesc.count; ++i)
				frameBufferAttachments.push_back(m_colorTargets[i]->getVkImageView());
		}

		if (m_depthTarget)
			frameBufferAttachments.push_back(m_depthTarget->getVkImageView());
		else if (m_setDesc.usingPrimaryDepthStencil)
			frameBufferAttachments.push_back(primaryDepthTarget->getVkImageView());

		VkFramebufferCreateInfo frameBufferCreateInfo = {};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCreateInfo.renderPass = rt.renderPass;
		frameBufferCreateInfo.attachmentCount = (uint32_t)frameBufferAttachments.size();
		frameBufferCreateInfo.pAttachments = frameBufferAttachments.ptr();
		frameBufferCreateInfo.width = m_setDesc.width;
		frameBufferCreateInfo.height = m_setDesc.height;
		frameBufferCreateInfo.layers = 1;
		if (vkCreateFramebuffer(m_logicalDevice, &frameBufferCreateInfo, nullptr, &rt.frameBuffer) != VK_SUCCESS)
			return false;
	}

	T_ASSERT(rt.renderPass != nullptr);
	T_ASSERT(rt.frameBuffer != nullptr);

	if (colorIndex >= 0)
		m_colorTargets[colorIndex]->prepareAsTarget(commandBuffer);
	else
	{
		for (auto colorTarget : m_colorTargets)
			colorTarget->prepareAsTarget(commandBuffer);
	}

	if (m_depthTarget)
		m_depthTarget->prepareAsTarget(commandBuffer);

	AlignedVector< VkClearValue > clearValues;

	if (colorIndex >= 0)
	{
		auto& cv = clearValues.push_back();
		clear.colors[0].storeUnaligned(cv.color.float32);
	}
	else
	{
		for (int32_t i = 0; i < m_setDesc.count; ++i)
		{
			auto& cv = clearValues.push_back();
			clear.colors[i].storeUnaligned(cv.color.float32);
		}
	}
	if (m_depthTarget || m_setDesc.usingPrimaryDepthStencil)
	{
		auto& cv = clearValues.push_back();
		cv.depthStencil.depth = clear.depth;
		cv.depthStencil.stencil = clear.stencil;
	}

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = rt.renderPass;
	renderPassBeginInfo.framebuffer = rt.frameBuffer;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = m_setDesc.width;
	renderPassBeginInfo.renderArea.extent.height = m_setDesc.height;
	renderPassBeginInfo.clearValueCount = (uint32_t)clearValues.size();
	renderPassBeginInfo.pClearValues = clearValues.c_ptr();
	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo,  VK_SUBPASS_CONTENTS_INLINE);

	outId = rt.id;
	outRenderPass = rt.renderPass;

	return true;
}

bool RenderTargetSetVk::prepareAsTexture(VkCommandBuffer commandBuffer)
{
	for (int32_t i = 0; i < m_setDesc.count; ++i)
		m_colorTargets[i]->prepareAsTexture(commandBuffer);

	if (m_depthTarget)
		m_depthTarget->prepareAsTexture(commandBuffer);

	return true;
}

	}
}
