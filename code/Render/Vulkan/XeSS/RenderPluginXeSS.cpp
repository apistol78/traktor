/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Vulkan/XeSS/RenderPluginXeSS.h"

#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/RenderTargetVk.h"
#include "Render/Vulkan/RenderViewVk.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderPluginXeSS", 0, RenderPluginXeSS, IRenderPlugin)

void RenderPluginXeSS::getExtensions(AlignedVector< const char* >& outExtensions)
{
	uint32_t extensionCount;
	const char* const* extensions;
	uint32_t minVKApiVersion;

	auto status = xessVKGetRequiredInstanceExtensions(&extensionCount, &extensions, &minVKApiVersion);
	if (status == XESS_RESULT_SUCCESS)
		for (size_t extensionIdx = 0; extensionIdx < extensionCount; ++extensionIdx)
			outExtensions.push_back(extensions[extensionIdx]);
}

void RenderPluginXeSS::getDeviceExtensions(VkInstance instance, VkPhysicalDevice physicalDevice, AlignedVector< const char* >& outExtensions)
{
	uint32_t extensionCount;
	const char* const* extensions;

	auto status = xessVKGetRequiredDeviceExtensions(instance, physicalDevice, &extensionCount, &extensions);
	if (status == XESS_RESULT_SUCCESS)
		for (size_t extensionIdx = 0; extensionIdx < extensionCount; ++extensionIdx)
			outExtensions.push_back(extensions[extensionIdx]);
}

void RenderPluginXeSS::destroy()
{
}

void RenderPluginXeSS::render(IRenderView* renderView, ITexture* colorTexture, ITexture* depthTexture, ITexture* velocityTexture, ITexture* outputTexture, const Vector4& jitter)
{
	RenderViewVk* rv = mandatory_non_null_type_cast< RenderViewVk* >(renderView);
	Context* ctx = rv->getContext();
	xess_result_t result;

	if (m_xessContext == nullptr)
	{
		result = xessVKCreateContext(ctx->getInstance(), ctx->getPhysicalDevice(), ctx->getLogicalDevice(), &m_xessContext);
		if (result != XESS_RESULT_SUCCESS)
			return;

		m_initWidth = m_initHeight = -1;
	}

	const ITexture::Size inputSize = colorTexture->getSize();
	const ITexture::Size outputSize = outputTexture->getSize();

	if (outputSize.x != m_initWidth || outputSize.y != m_initHeight)
	{
		const xess_vk_init_params_t params = {
			/* Output width and height */
			{ outputSize.x, outputSize.y },
			/* Quality setting */
			XESS_QUALITY_SETTING_AA,
			/* Initialization flags. */
			XESS_INIT_FLAG_HIGH_RES_MV,
			/* Specfies the node mask for internally created resources on
			 * multi-adapter systems. */
			0,
			/* Specfies the node visibility mask for internally created resources
			 * on multi-adapter systems. */
			0,
			/* Optional externally allocated buffers storage for XeSS-SR. If NULL the
			 * storage is allocated internally. If allocated, the heap type must be
			 * D3D12_HEAP_TYPE_DEFAULT. This heap is not accessed by the CPU. */
			nullptr,
			/* Offset in the externally allocated heap for temporary buffers storage. */
			0,
			/* Optional externally allocated textures storage for XeSS-SR. If NULL the
			 * storage is allocated internally. If allocated, the heap type must be
			 * D3D12_HEAP_TYPE_DEFAULT. This heap is not accessed by the CPU. */
			nullptr,
			/* Offset in the externally allocated heap for temporary textures storage. */
			0,
			/* No pipeline library */
			NULL
		};

		result = xessVKInit(m_xessContext, &params);
		if (result != XESS_RESULT_SUCCESS)
			return;

		// NDC to pixels scaling factors.
		xessSetVelocityScale(m_xessContext, inputSize.x / 2.0f, -inputSize.y / 2.0f);
		xessSetJitterScale(m_xessContext, inputSize.x / 2.0f, -inputSize.y / 2.0f);

		m_initWidth = outputSize.x;
		m_initHeight = outputSize.y;
	}

	xess_vk_execute_params_t exec_params{};
	exec_params.inputWidth = inputSize.x;
	exec_params.inputHeight = inputSize.y;
	exec_params.jitterOffsetX = jitter.z();
	exec_params.jitterOffsetY = jitter.w();
	exec_params.exposureScale = 1.0f;

	exec_params.colorTexture = {
		.imageView = static_cast< RenderTargetVk* >(colorTexture)->getImageTarget()->getVkImageView(),
		.image = static_cast< RenderTargetVk* >(colorTexture)->getImageTarget()->getVkImage(),
		.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0u, 1u, 0u, 1u },
		.format = static_cast< RenderTargetVk* >(colorTexture)->getVkFormat(),
		.width = (unsigned int)inputSize.x,
		.height = (unsigned int)inputSize.y
	};
	exec_params.velocityTexture = {
		.imageView = static_cast< RenderTargetVk* >(velocityTexture)->getImageTarget()->getVkImageView(),
		.image = static_cast< RenderTargetVk* >(velocityTexture)->getImageTarget()->getVkImage(),
		.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0u, 1u, 0u, 1u },
		.format = static_cast< RenderTargetVk* >(velocityTexture)->getVkFormat(),
		.width = (unsigned int)inputSize.x,
		.height = (unsigned int)inputSize.y
	};
	exec_params.depthTexture = {
		.imageView = static_cast< RenderTargetVk* >(depthTexture)->getImageTarget()->getVkImageView(),
		.image = static_cast< RenderTargetVk* >(depthTexture)->getImageTarget()->getVkImage(),
		.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0u, 1u, 0u, 1u },
		.format = static_cast< RenderTargetVk* >(depthTexture)->getVkFormat(),
		.width = (unsigned int)inputSize.x,
		.height = (unsigned int)inputSize.y
	};
	exec_params.outputTexture = {
		.imageView = static_cast< RenderTargetVk* >(outputTexture)->getImageTarget()->getVkImageView(),
		.image = static_cast< RenderTargetVk* >(outputTexture)->getImageTarget()->getVkImage(),
		.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0u, 1u, 0u, 1u },
		.format = static_cast< RenderTargetVk* >(outputTexture)->getVkFormat(),
		.width = (unsigned int)outputSize.x,
		.height = (unsigned int)outputSize.y
	};
	exec_params.exposureScaleTexture.image = nullptr;
	exec_params.exposureScaleTexture.imageView = nullptr;

	CommandBuffer* commandBuffer = rv->getGraphicsCommandBuffer();
	result = xessVKExecute(m_xessContext, *commandBuffer, &exec_params);

	rv->barrier(Stage::Compute, Stage::Fragment, nullptr, 0);
}

}
