#pragma optimize( "", off )

#include <limits>
#include "Core/Containers/AlignedVector.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/Types.h"
#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/RenderTargetDepthVk.h"
#include "Render/Vulkan/RenderTargetVk.h"
#include "Render/Vulkan/RenderTargetSetVk.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

uint32_t s_nextId = 1;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSetVk", RenderTargetSetVk, RenderTargetSet)

RenderTargetSetVk::RenderTargetSetVk()
:	RenderTargetSet()
,	m_renderPass(nullptr)
,	m_frameBuffer(nullptr)
,	m_lastColorIndex(std::numeric_limits< int32_t >::max())
,	m_lastClearMask(0)
,	m_id(s_nextId++)
{
}

RenderTargetSetVk::~RenderTargetSetVk()
{
	destroy();
}

bool RenderTargetSetVk::createPrimary(VkPhysicalDevice physicalDevice, VkDevice device, int32_t width, int32_t height, VkFormat colorFormat, VkImage colorImage, VkFormat depthFormat, VkImage depthImage)
{
	m_colorTargets.resize(1);
	m_colorTargets[0] = new RenderTargetVk();
	if (!m_colorTargets[0]->createPrimary(physicalDevice, device, width, height, colorFormat, colorImage))
		return false;

	m_depthTarget = new RenderTargetDepthVk();
	if (!m_depthTarget->createPrimary(physicalDevice, device, width, height, depthFormat, depthImage))
		return false;

	//VkAttachmentDescription passAttachments[2] = {};
	//passAttachments[0].format = depthFormat;
	//passAttachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	//passAttachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	//passAttachments[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	//passAttachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	//passAttachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	//passAttachments[0].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	//passAttachments[0].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	//passAttachments[1].format = colorFormat;
	//passAttachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	//passAttachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	//passAttachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	//passAttachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	//passAttachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	//passAttachments[1].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	//passAttachments[1].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//VkAttachmentReference colorAttachmentReference = {};
	//colorAttachmentReference.attachment = 0;
	//colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//VkAttachmentReference depthAttachmentReference = {};
	//depthAttachmentReference.attachment = 1;
	//depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	//VkSubpassDescription subpass = {};
	//subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	//subpass.colorAttachmentCount = 1;
	//subpass.pColorAttachments = &colorAttachmentReference;
	//subpass.pDepthStencilAttachment = &depthAttachmentReference;

	//VkRenderPassCreateInfo renderPassCreateInfo = {};
	//renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	//renderPassCreateInfo.attachmentCount = 2;
	//renderPassCreateInfo.pAttachments = passAttachments;
	//renderPassCreateInfo.subpassCount = 1;
	//renderPassCreateInfo.pSubpasses = &subpass;
	//if (vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &m_renderPass) != VK_SUCCESS)
	//	return false;

 //	VkImageView frameBufferAttachments[2];
	//frameBufferAttachments[0] = m_depthTarget->getVkImageView();
	//frameBufferAttachments[1] = m_colorTargets[0]->getVkImageView();

	//VkFramebufferCreateInfo frameBufferCreateInfo = {};
	//frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	//frameBufferCreateInfo.renderPass = m_renderPass;
	//frameBufferCreateInfo.attachmentCount = 2;
	//frameBufferCreateInfo.pAttachments = frameBufferAttachments;
	//frameBufferCreateInfo.width = width;
	//frameBufferCreateInfo.height = height;
	//frameBufferCreateInfo.layers = 1;
	//if (vkCreateFramebuffer(device, &frameBufferCreateInfo, nullptr, &m_frameBuffer) != VK_SUCCESS)
	//	return false;

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

bool RenderTargetSetVk::create(VkPhysicalDevice physicalDevice, VkDevice device, const RenderTargetSetCreateDesc& setDesc)
{
	m_colorTargets.resize(setDesc.count);
	for (int32_t i = 0; i < setDesc.count; ++i)
	{
		m_colorTargets[i] = new RenderTargetVk();
		if (!m_colorTargets[i]->create(physicalDevice, device, setDesc, setDesc.targets[i]))
			return false;
	}

	if (setDesc.createDepthStencil)
	{
		m_depthTarget = new RenderTargetDepthVk();
		if (!m_depthTarget->create(physicalDevice, device, setDesc))
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

void RenderTargetSetVk::swap(int32_t index1, int32_t index2)
{
#if defined(_WIN32)
	std::swap(m_colorTargets[index1], m_colorTargets[index2]);
#endif
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
	return false;
}

bool RenderTargetSetVk::prepareAsTarget(
	VkDevice device,
	VkCommandBuffer commandBuffer,
	int32_t colorIndex,
	uint32_t clearMask,
	const Color4f* colors,
	float depth,
	int32_t stencil,
	RenderTargetDepthVk* primaryDepthTarget
)
{
	// Do we need to nuke previous render pass and framebuffer?
	if (m_lastColorIndex != colorIndex || m_lastClearMask != clearMask)
	{
		m_renderPass = nullptr;
		m_frameBuffer = nullptr;
		m_lastColorIndex = colorIndex;
		m_lastClearMask = clearMask;
	}

	// (Re-)create render pass.
	if (!m_renderPass)
	{
		AlignedVector< VkAttachmentDescription > passAttachments;

		if (colorIndex >= 0)
		{
			// One color target selected.
			VkAttachmentDescription passAttachment = {};
			passAttachment.format = m_colorTargets[colorIndex]->getVkFormat();
			passAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			passAttachment.loadOp = ((clearMask & CfColor) != 0) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;// VK_ATTACHMENT_LOAD_OP_DONT_CARE;
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
			for (int i = 0; i < m_setDesc.count; ++i)
			{
				VkAttachmentDescription passAttachment = {};
				passAttachment.format = m_colorTargets[i]->getVkFormat();
				passAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
				passAttachment.loadOp = ((clearMask & CfColor) != 0) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;// VK_ATTACHMENT_LOAD_OP_DONT_CARE;
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
			passAttachment.loadOp = ((clearMask & CfDepth) != 0) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;// VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			passAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;// VK_ATTACHMENT_STORE_OP_DONT_CARE;
			passAttachment.stencilLoadOp = ((clearMask & CfStencil) != 0) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;// VK_ATTACHMENT_LOAD_OP_DONT_CARE;
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
			passAttachment.loadOp = ((clearMask & CfDepth) != 0) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;// VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			passAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;//VK_ATTACHMENT_STORE_OP_DONT_CARE;
			passAttachment.stencilLoadOp = ((clearMask & CfStencil) != 0) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;// VK_ATTACHMENT_LOAD_OP_DONT_CARE;
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
		if (vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &m_renderPass) != VK_SUCCESS)
			return false;
	}

	// (Re-)create frame buffer.
	if (!m_frameBuffer)
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
		frameBufferCreateInfo.renderPass = m_renderPass;
		frameBufferCreateInfo.attachmentCount = (uint32_t)frameBufferAttachments.size();
		frameBufferCreateInfo.pAttachments = frameBufferAttachments.ptr();
		frameBufferCreateInfo.width = m_setDesc.width;
		frameBufferCreateInfo.height = m_setDesc.height;
		frameBufferCreateInfo.layers = 1;
		if (vkCreateFramebuffer(device, &frameBufferCreateInfo, nullptr, &m_frameBuffer) != VK_SUCCESS)
			return false;
	}

	T_ASSERT(m_renderPass != nullptr);
	T_ASSERT(m_frameBuffer != nullptr);

	if (colorIndex >= 0)
		m_colorTargets[colorIndex]->prepareAsTarget(commandBuffer);
	else
	{
		for (auto colorTarget : m_colorTargets)
			colorTarget->prepareAsTarget(commandBuffer);
	}

	AlignedVector< VkClearValue > clearValues;

	if (colorIndex >= 0)
	{
		auto& cv = clearValues.push_back();
		colors[0].storeUnaligned(cv.color.float32);
	}
	else
	{
		for (int32_t i = 0; i < m_setDesc.count; ++i)
		{
			auto& cv = clearValues.push_back();
			colors[i].storeUnaligned(cv.color.float32);
		}
	}
	if (m_depthTarget || m_setDesc.usingPrimaryDepthStencil)
	{
		auto& cv = clearValues.push_back();
		cv.depthStencil.depth = depth;
		cv.depthStencil.stencil = stencil;
	}

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = m_renderPass;
	renderPassBeginInfo.framebuffer = m_frameBuffer;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = m_setDesc.width;
	renderPassBeginInfo.renderArea.extent.height = m_setDesc.height;
	renderPassBeginInfo.clearValueCount = (uint32_t)clearValues.size();
	renderPassBeginInfo.pClearValues = clearValues.c_ptr();
	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

	return true;
}

bool RenderTargetSetVk::prepareAsTexture(VkCommandBuffer commandBuffer)
{
	for (int32_t i = 0; i < m_setDesc.count; ++i)
		m_colorTargets[i]->prepareAsTexture(commandBuffer);
	return true;
}

	}
}
