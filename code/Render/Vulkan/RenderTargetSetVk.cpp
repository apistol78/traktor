#include <cstring>
#include <limits>
#include "Core/Containers/StaticVector.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/Types.h"
#include "Render/Vulkan/RenderTargetDepthVk.h"
#include "Render/Vulkan/RenderTargetVk.h"
#include "Render/Vulkan/RenderTargetSetVk.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/Queue.h"
#include "Render/Vulkan/Private/Utilities.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

uint32_t s_nextId = 1;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSetVk", RenderTargetSetVk, IRenderTargetSet)

RenderTargetSetVk::RenderTargetSetVk(Context* context, uint32_t& instances)
:	m_context(context)
,	m_instances(instances)
{
	Atomic::increment((int32_t&)m_instances);
}

RenderTargetSetVk::~RenderTargetSetVk()
{
	destroy();
	Atomic::decrement((int32_t&)m_instances);
}

bool RenderTargetSetVk::createPrimary(
	int32_t width,
	int32_t height,
	uint32_t multiSample,
	VkFormat colorFormat,
	VkImage colorImage,
	RenderTargetDepthVk* depthTarget,
	const wchar_t* const tag
)
{
	m_colorTargets.resize(1);
	m_colorTargets[0] = new RenderTargetVk(m_context);
	if (!m_colorTargets[0]->createPrimary(width, height, multiSample, colorFormat, colorImage, tag))
		return false;

	m_depthTarget = depthTarget;
	m_depthTargetShared = true;

	m_setDesc.count = 1;
	m_setDesc.width = width;
	m_setDesc.height = height;
	m_setDesc.multiSample = multiSample;
	m_setDesc.createDepthStencil = true;
	m_setDesc.usingDepthStencilAsTexture = false;
	m_setDesc.usingPrimaryDepthStencil = false;	// We contain primary depth thus do not share.
	m_setDesc.ignoreStencil = false;
	m_setDesc.generateMips = false;
	m_setDesc.targets[0].format = TfR8G8B8A8;
	m_setDesc.targets[0].sRGB = false;

	return true;
}

bool RenderTargetSetVk::create(
	const RenderTargetSetCreateDesc& setDesc,
	IRenderTargetSet* sharedDepthStencil,
	const wchar_t* const tag
)
{
	m_colorTargets.resize(setDesc.count);
	for (int32_t i = 0; i < setDesc.count; ++i)
	{
		m_colorTargets[i] = new RenderTargetVk(m_context);
		if (!m_colorTargets[i]->create(setDesc, setDesc.targets[i], tag))
			return false;
	}

	if (setDesc.createDepthStencil)
	{
		m_depthTarget = new RenderTargetDepthVk(m_context);
		if (!m_depthTarget->create(setDesc, tag))
			return false;
		m_depthTargetShared = false;
	}
	else if (sharedDepthStencil != nullptr)
	{
		m_depthTarget = static_cast< RenderTargetSetVk* >(sharedDepthStencil)->getDepthTargetVk();
		m_depthTargetShared = true;
	}

	m_setDesc = setDesc;
	return true;
}

void RenderTargetSetVk::destroy()
{
	// Destroy color target textures.
	for (auto colorTarget : m_colorTargets)
	{
		if (colorTarget)
			colorTarget->destroy();
	}
	m_colorTargets.resize(0);

	// Only destroy depth target texture if not being shared, else just release reference.
	if (!m_depthTargetShared)
		safeDestroy(m_depthTarget);
	else
		m_depthTarget = nullptr;
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
	return index < (int32_t)m_colorTargets.size() ? m_colorTargets[index] : nullptr;
}

ISimpleTexture* RenderTargetSetVk::getDepthTexture() const
{
	return m_depthTarget;
}

bool RenderTargetSetVk::isContentValid() const
{
	return true;
}

bool RenderTargetSetVk::read(int32_t index, void* buffer) const
{
	//if (index < 0 || index >= (int32_t)m_colorTargets.size())
	//	return false;

	//// Create offscreen image copy.
	//VkImage hostImage = 0;

	//VkImageCreateInfo ici = {};
	//ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	//ici.imageType = VK_IMAGE_TYPE_2D;
	//ici.format = m_colorTargets[index]->getVkFormat();
	//ici.extent.width = m_setDesc.width;
	//ici.extent.height = m_setDesc.height;
	//ici.extent.depth = 1;
	//ici.arrayLayers = 1;
	//ici.mipLevels = 1;
	//ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	//ici.samples = VK_SAMPLE_COUNT_1_BIT;
	//ici.tiling = VK_IMAGE_TILING_LINEAR;
	//ici.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	//if (vkCreateImage(m_context->getLogicalDevice(), &ici, nullptr, &hostImage) != VK_SUCCESS)
	//	return false;

	//VkMemoryRequirements memoryRequirements;
	//vkGetImageMemoryRequirements(m_context->getLogicalDevice(), hostImage, &memoryRequirements);

	//VkDeviceMemory hostImageMemory = 0;

	//VkMemoryAllocateInfo mai = {};
	//mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	//mai.allocationSize = memoryRequirements.size;
	//mai.memoryTypeIndex = getMemoryTypeIndex(m_context->getPhysicalDevice(), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memoryRequirements);

	//vkAllocateMemory(m_context->getLogicalDevice(), &mai, nullptr, &hostImageMemory);
	//vkBindImageMemory(m_context->getLogicalDevice(), hostImage, hostImageMemory, 0);

	//auto commandBuffer = m_context->getGraphicsQueue()->acquireCommandBuffer(T_FILE_LINE_W);

	//// Transfer color target into host image.
	//VkImageMemoryBarrier imb = {};
	//imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	//imb.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	//imb.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	//imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	//imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	//imb.image = hostImage;
	//imb.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	//imb.subresourceRange.baseMipLevel = 0;
	//imb.subresourceRange.levelCount = 1;
	//imb.subresourceRange.baseArrayLayer = 0;
	//imb.subresourceRange.layerCount = 1;
	//imb.srcAccessMask = 0;
	//imb.dstAccessMask = 0;
	//vkCmdPipelineBarrier(
	//	*commandBuffer,
	//	VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
	//	VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
	//	0,
	//	0, nullptr,
	//	0, nullptr,
	//	1, &imb
	//);

	//// Convert target for optimal read.
	//m_colorTargets[index]->prepareForReadBack(commandBuffer);

	//// Copy target into host image.
	//VkImageCopy ic = {};
	//ic.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	//ic.srcSubresource.layerCount = 1;
	//ic.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	//ic.dstSubresource.layerCount = 1;
	//ic.extent.width = m_setDesc.width;
	//ic.extent.height = m_setDesc.height;
	//ic.extent.depth = 1;
	//vkCmdCopyImage(
	//	*commandBuffer,
	//	m_colorTargets[index]->getVkImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	//	hostImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	//	1,
	//	&ic
	//);

	//// Convert host image into general layout; must be general to be mappable.
	//imb = {};
	//imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	//imb.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	//imb.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	//imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	//imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	//imb.image = hostImage;
	//imb.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	//imb.subresourceRange.baseMipLevel = 0;
	//imb.subresourceRange.levelCount = 1;
	//imb.subresourceRange.baseArrayLayer = 0;
	//imb.subresourceRange.layerCount = 1;
	//imb.srcAccessMask = 0;
	//imb.dstAccessMask = 0;
	//vkCmdPipelineBarrier(
	//	*commandBuffer,
	//	VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
	//	VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
	//	0,
	//	0, nullptr,
	//	0, nullptr,
	//	1, &imb
	//);

	//// Submit commands, and wait until finished.
	//commandBuffer->submitAndWait();

	//// Get information about image.
	//VkImageSubresource isr = {};
	//isr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	//VkSubresourceLayout srl = {};
	//vkGetImageSubresourceLayout(m_context->getLogicalDevice(), hostImage, &isr, &srl);

	//// Read back data.
	//uint8_t* src = nullptr;
	//vkMapMemory(m_context->getLogicalDevice(), hostImageMemory, 0, VK_WHOLE_SIZE, 0, (void**)&src);
	//src += srl.offset;

	//uint32_t fragmentSize = 4 * sizeof(float);

	//uint8_t* dst = (uint8_t*)buffer;
	//for (int32_t y = 0; y < m_setDesc.height; ++y)
	//{
	//	std::memcpy(dst, src, m_setDesc.width * fragmentSize);
	//	dst += m_setDesc.width * fragmentSize;
	//	src += srl.rowPitch;
	//}

	//vkUnmapMemory(m_context->getLogicalDevice(), hostImageMemory);
	//
	//// Cleanup
	//vkFreeMemory(m_context->getLogicalDevice(), hostImageMemory, 0);
	//vkDestroyImage(m_context->getLogicalDevice(), hostImage, 0);
	//return true;

	return false;
}

void RenderTargetSetVk::setDebugName(const wchar_t* name)
{
	//for (auto colorTarget : m_colorTargets)
	//	setObjectDebugName(m_context->getLogicalDevice(), name, (uint64_t)colorTarget->getVkImage(), VK_OBJECT_TYPE_IMAGE);

	//if (m_depthTarget)
	//	setObjectDebugName(m_context->getLogicalDevice(), name, (uint64_t)m_depthTarget->getVkImage(), VK_OBJECT_TYPE_IMAGE);
}

bool RenderTargetSetVk::prepareAsTarget(
	CommandBuffer* commandBuffer,
	int32_t colorIndex,
	const Clear& clear,
	uint32_t load,
	uint32_t store,
	RenderTargetDepthVk* primaryDepthTarget,
	uint32_t& outId,
	VkRenderPass& outRenderPass,
	VkFramebuffer& outFrameBuffer
)
{
	const VkSampleCountFlagBits sampleCount = (m_setDesc.multiSample <= 1) ? VK_SAMPLE_COUNT_1_BIT : (VkSampleCountFlagBits)m_setDesc.multiSample;

	auto key = std::make_tuple(
		colorIndex,
		clear.mask
	);

	auto& rt = m_renderPasses[key];

	if (rt.renderPass == 0)
	{
		rt.id = s_nextId++;

		StaticVector< VkAttachmentDescription, RenderTargetSetCreateDesc::MaxTargets * 2 + 1 > passAttachments;

		if (colorIndex >= 0)
		{
			// One color target selected.
			VkAttachmentDescription pa = {};
			pa.format = m_colorTargets[colorIndex]->getVkFormat();
			pa.samples = sampleCount;

			if ((clear.mask & CfColor) != 0)
				pa.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			else if ((load & TfColor) != 0)
				pa.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			else
				pa.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			
			if ((store & TfColor) != 0)
				pa.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			else
				pa.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

			// Not used in color targets.
			pa.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			pa.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

			if ((clear.mask & CfColor) != 0)
				pa.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;	// If we're clearing let's also assume we don't know about layout.
			else
				pa.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;	// Must keep last layout since we're loading existing color.

			pa.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			passAttachments.push_back(pa);

			// Add resolve targets.
			if (needResolve())
			{
				VkAttachmentDescription pa = {};
				pa.format = m_colorTargets[colorIndex]->getVkFormat();
				pa.samples = VK_SAMPLE_COUNT_1_BIT;
				pa.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				pa.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				pa.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				pa.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				pa.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				pa.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				passAttachments.push_back(pa);
			}
		}
		else
		{
			// Attach all color targets for MRT.
			for (int32_t i = 0; i < m_setDesc.count; ++i)
			{
				VkAttachmentDescription pa = {};
				pa.format = m_colorTargets[i]->getVkFormat();
				pa.samples = sampleCount;

				if ((clear.mask & CfColor) != 0)
					pa.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				else if ((load & TfColor) != 0)
					pa.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
				else
					pa.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			
				if ((store & TfColor) != 0)
					pa.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				else
					pa.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

				// Not used in color targets.
				pa.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				pa.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				
				if ((clear.mask & CfColor) != 0)
					pa.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;	// If we're clearing let's also assume we don't know about layout.
				else
					pa.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;	// Must keep last layout since we're loading existing color.
			
				pa.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				passAttachments.push_back(pa);

				// Add resolve targets.
				if (needResolve())
				{
					VkAttachmentDescription pa = {};
					pa.format = m_colorTargets[i]->getVkFormat();
					pa.samples = VK_SAMPLE_COUNT_1_BIT;
					pa.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
					pa.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
					pa.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
					pa.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
					pa.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
					pa.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					passAttachments.push_back(pa);
				}
			}
		}

		if (m_depthTarget || m_setDesc.usingPrimaryDepthStencil)
		{
			VkAttachmentDescription pa = {};

			if (m_depthTarget)
				pa.format = m_depthTarget->getVkFormat();
			else
				pa.format = primaryDepthTarget->getVkFormat();

			pa.samples = sampleCount;

			if ((clear.mask & CfDepth) != 0)
				pa.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			else if ((load & TfDepth) != 0)
				pa.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			else
				pa.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			
			if ((store & TfDepth) != 0)
				pa.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			else
				pa.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

			if ((clear.mask & CfStencil) != 0)
				pa.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			else if ((load & TfDepth) != 0)
				pa.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			else
				pa.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

			if ((store & TfDepth) != 0)
				pa.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
			else
				pa.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

			pa.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			pa.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			passAttachments.push_back(pa);
		}

		StaticVector< VkAttachmentReference, RenderTargetSetCreateDesc::MaxTargets > colorAttachmentReferences;
		StaticVector< VkAttachmentReference, RenderTargetSetCreateDesc::MaxTargets > resolveAttachmentReferences;
		if (colorIndex >= 0)
		{
			auto& car = colorAttachmentReferences.push_back();
			car.attachment = 0;
			car.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			if (needResolve())
			{
				auto& rar = resolveAttachmentReferences.push_back();
				rar.attachment = 1;
				rar.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}
		}
		else
		{
			if (needResolve())
			{
				for (int i = 0; i < m_setDesc.count; ++i)
				{
					auto& car = colorAttachmentReferences.push_back();
					car.attachment = i * 2;
					car.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

					auto& rar = resolveAttachmentReferences.push_back();
					rar.attachment = i * 2 + 1;
					rar.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				}
			}
			else
			{
				for (int i = 0; i < m_setDesc.count; ++i)
				{
					auto& car = colorAttachmentReferences.push_back();
					car.attachment = i;
					car.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				}
			}
		}

		// Depth attachment is always last.
		VkAttachmentReference depthAttachmentReference = {};
		depthAttachmentReference.attachment = passAttachments.size() - 1;
		depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = (uint32_t)colorAttachmentReferences.size();
		subpass.pColorAttachments = colorAttachmentReferences.c_ptr();
		if (!resolveAttachmentReferences.empty())
			subpass.pResolveAttachments = resolveAttachmentReferences.c_ptr();
		if (m_depthTarget || m_setDesc.usingPrimaryDepthStencil)
			subpass.pDepthStencilAttachment = &depthAttachmentReference;

		VkRenderPassCreateInfo rpci = {};
		rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		rpci.attachmentCount = (uint32_t)passAttachments.size();
		rpci.pAttachments = passAttachments.ptr();
		rpci.subpassCount = 1;
		rpci.pSubpasses = &subpass;
		if (vkCreateRenderPass(m_context->getLogicalDevice(), &rpci, nullptr, &rt.renderPass) != VK_SUCCESS)
			return false;
	}

	// (Re-)create frame buffer.
	if (rt.frameBuffer == 0)
	{
 		AlignedVector< VkImageView > fba;
		
		if (colorIndex >= 0)
		{
			fba.push_back(m_colorTargets[colorIndex]->getImageTarget()->getVkImageView());
			if (needResolve())
				fba.push_back(m_colorTargets[colorIndex]->getImageResolved()->getVkImageView());
		}
		else
		{
			for (int32_t i = 0; i < m_setDesc.count; ++i)
			{
				fba.push_back(m_colorTargets[i]->getImageTarget()->getVkImageView());
				if (needResolve())
					fba.push_back(m_colorTargets[i]->getImageResolved()->getVkImageView());
			}
		}

		if (m_depthTarget)
			fba.push_back(m_depthTarget->getImage()->getVkImageView());
		else if (m_setDesc.usingPrimaryDepthStencil)
			fba.push_back(primaryDepthTarget->getImage()->getVkImageView());

		VkFramebufferCreateInfo fbci = {};
		fbci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbci.renderPass = rt.renderPass;
		fbci.attachmentCount = (uint32_t)fba.size();
		fbci.pAttachments = fba.ptr();
		fbci.width = m_setDesc.width;
		fbci.height = m_setDesc.height;
		fbci.layers = 1;
		if (vkCreateFramebuffer(m_context->getLogicalDevice(), &fbci, nullptr, &rt.frameBuffer) != VK_SUCCESS)
			return false;
	}

	T_ASSERT(rt.renderPass != 0);
	T_ASSERT(rt.frameBuffer != 0);

	if (colorIndex >= 0)
		m_colorTargets[colorIndex]->prepareAsTarget(commandBuffer);
	else
	{
		for (auto colorTarget : m_colorTargets)
			colorTarget->prepareAsTarget(commandBuffer);
	}

	if (m_depthTarget)
		m_depthTarget->prepareAsTarget(commandBuffer);

	outId = rt.id;
	outRenderPass = rt.renderPass;
	outFrameBuffer = rt.frameBuffer;
	return true;
}

bool RenderTargetSetVk::prepareAsTexture(
	CommandBuffer* commandBuffer,
	int32_t colorIndex
)
{
	if (colorIndex >= 0)
		m_colorTargets[colorIndex]->prepareAsTexture(commandBuffer);
	else
	{
		for (int32_t i = 0; i < m_setDesc.count; ++i)
			m_colorTargets[i]->prepareAsTexture(commandBuffer);
	}
	
	if (m_depthTarget && m_setDesc.usingDepthStencilAsTexture)
		m_depthTarget->prepareAsTexture(commandBuffer);

	return true;
}

	}
}
