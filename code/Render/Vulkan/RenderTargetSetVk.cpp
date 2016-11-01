#include "Core/Containers/AlignedVector.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/Types.h"
#include "Render/Vulkan/RenderTargetDepthVk.h"
#include "Render/Vulkan/RenderTargetVk.h"
#include "Render/Vulkan/RenderTargetSetVk.h"
#if defined(_WIN32)
#	include "Render/Vulkan/Win32/ApiLoader.h"
#endif

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSetVk", RenderTargetSetVk, RenderTargetSet)

RenderTargetSetVk::RenderTargetSetVk()
:	RenderTargetSet()
,	m_width(0)
,	m_height(0)
,	m_renderPass(0)
,	m_framebuffer(0)
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

	VkAttachmentDescription passAttachments[2] = {};
	passAttachments[0].format = colorFormat;
	passAttachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	passAttachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	passAttachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	passAttachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	passAttachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	passAttachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	passAttachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	passAttachments[1].format = depthFormat;
	passAttachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	passAttachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	passAttachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	passAttachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	passAttachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	passAttachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	passAttachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
 
	VkAttachmentReference colorAttachmentReference = {};
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
 
	VkAttachmentReference depthAttachmentReference = {};
	depthAttachmentReference.attachment = 1;
	depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentReference;
	subpass.pDepthStencilAttachment = &depthAttachmentReference;
 
	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = 2;
	renderPassCreateInfo.pAttachments = passAttachments;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;
	if (vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &m_renderPass) != VK_SUCCESS)
		return false;

 	VkImageView frameBufferAttachments[2];
	frameBufferAttachments[0] = m_colorTargets[0]->getVkImageView();
	frameBufferAttachments[1] = m_depthTarget->getVkImageView();

	VkFramebufferCreateInfo frameBufferCreateInfo = {};
	frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frameBufferCreateInfo.renderPass = m_renderPass;
	frameBufferCreateInfo.attachmentCount = 2;
	frameBufferCreateInfo.pAttachments = frameBufferAttachments;
	frameBufferCreateInfo.width = width;
	frameBufferCreateInfo.height = height;
	frameBufferCreateInfo.layers = 1;
	if (vkCreateFramebuffer(device, &frameBufferCreateInfo, nullptr, &m_framebuffer) != VK_SUCCESS)
		return false;

	m_width = width;
	m_height = height;

	return true;
}

bool RenderTargetSetVk::create(VkPhysicalDevice physicalDevice, VkDevice device, const RenderTargetSetCreateDesc& setDesc)
{
	m_colorTargets.resize(setDesc.count);
	for (int i = 0; i < setDesc.count; ++i)
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

	AlignedVector< VkAttachmentDescription > passAttachments;
	for (int i = 0; i < setDesc.count; ++i)
	{
		VkAttachmentDescription passAttachment = {};
		passAttachment.format = m_colorTargets[i]->getVkFormat();
		passAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		passAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		passAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		passAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		passAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		passAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		passAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		passAttachments.push_back(passAttachment);
	}

	if (m_depthTarget)
	{
		VkAttachmentDescription passAttachment = {};
		passAttachment.format = m_depthTarget->getVkFormat();
		passAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		passAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		passAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		passAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		passAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		passAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		passAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		passAttachments.push_back(passAttachment);
	}

	VkAttachmentReference colorAttachmentReference = {};
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
 
	VkAttachmentReference depthAttachmentReference = {};
	depthAttachmentReference.attachment = 1;
	depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentReference;
	if (m_depthTarget)
		subpass.pDepthStencilAttachment = &depthAttachmentReference;
 
	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = passAttachments.size();
	renderPassCreateInfo.pAttachments = passAttachments.ptr();
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;
	if (vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &m_renderPass) != VK_SUCCESS)
		return false;

 	AlignedVector< VkImageView > frameBufferAttachments;
	for (int i = 0; i < setDesc.count; ++i)
		frameBufferAttachments.push_back(m_colorTargets[i]->getVkImageView());
	if (m_depthTarget)
		frameBufferAttachments.push_back(m_depthTarget->getVkImageView());

	VkFramebufferCreateInfo frameBufferCreateInfo = {};
	frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frameBufferCreateInfo.renderPass = m_renderPass;
	frameBufferCreateInfo.attachmentCount = frameBufferAttachments.size();
	frameBufferCreateInfo.pAttachments = frameBufferAttachments.ptr();
	frameBufferCreateInfo.width = setDesc.width;
	frameBufferCreateInfo.height = setDesc.height;
	frameBufferCreateInfo.layers = 1;
	if (vkCreateFramebuffer(device, &frameBufferCreateInfo, nullptr, &m_framebuffer) != VK_SUCCESS)
		return false;

	m_width = setDesc.width;
	m_height = setDesc.height;

	return true;
}

void RenderTargetSetVk::destroy()
{
	for (RefArray< RenderTargetVk >::iterator i = m_colorTargets.begin(); i != m_colorTargets.end(); ++i)
	{
		if (*i)
			(*i)->destroy();
	}
	m_colorTargets.resize(0);
	safeDestroy(m_depthTarget);
}

int RenderTargetSetVk::getWidth() const
{
	return m_width;
}

int RenderTargetSetVk::getHeight() const
{
	return m_height;
}

ISimpleTexture* RenderTargetSetVk::getColorTexture(int index) const
{
	return index < int(m_colorTargets.size()) ? m_colorTargets[index] : 0;
}

ISimpleTexture* RenderTargetSetVk::getDepthTexture() const
{
	return m_depthTarget;
}

void RenderTargetSetVk::swap(int index1, int index2)
{
	std::swap(m_colorTargets[index1], m_colorTargets[index2]);
}

void RenderTargetSetVk::discard()
{
}

bool RenderTargetSetVk::read(int index, void* buffer) const
{
	return false;
}

	}
}
