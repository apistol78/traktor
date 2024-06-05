/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Matrix44.h"
#include "Core/Thread/Atomic.h"
#include "Render/Vulkan/BufferViewVk.h"
#include "Render/Vulkan/ProgramVk.h"
#include "Render/Vulkan/ProgramResourceVk.h"
#include "Render/Vulkan/RenderTargetDepthVk.h"
#include "Render/Vulkan/RenderTargetVk.h"
#include "Render/Vulkan/TextureVk.h"
#include "Render/Vulkan/Private/ApiBuffer.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/Image.h"
#include "Render/Vulkan/Private/PipelineLayoutCache.h"
#include "Render/Vulkan/Private/ShaderModuleCache.h"
#include "Render/Vulkan/Private/Utilities.h"

#undef max

namespace traktor::render
{
	namespace
	{

VkShaderStageFlags getShaderStageFlags(uint8_t resourceStages)
{
	VkShaderStageFlags flags = 0;
	if (resourceStages & ProgramResourceVk::BsVertex)
		flags |= VK_SHADER_STAGE_VERTEX_BIT;
	if (resourceStages & ProgramResourceVk::BsFragment)
		flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
	if (resourceStages & ProgramResourceVk::BsCompute)
		flags |= VK_SHADER_STAGE_COMPUTE_BIT;
	T_ASSERT(flags != 0);
	return flags;
}

std::wstring getShaderStageNames(uint8_t resourceStages)
{
	std::wstring names = L"";
	if (resourceStages & ProgramResourceVk::BsVertex)
		names += L"+vertex";
	if (resourceStages & ProgramResourceVk::BsFragment)
		names += L"+fragment";
	if (resourceStages & ProgramResourceVk::BsCompute)
		names += L"+compute";
	return names;
}

bool storeIfNotEqual(const float* source, int32_t length, float* dest)
{
	for (int32_t i = 0; i < length; ++i)
	{
		if (dest[i] != source[i])
		{
			for (; i < length; ++i)
				dest[i] = source[i];
			return true;
		}
	}
	return false;
}

bool storeIfNotEqual(const Vector4* source, int32_t length, float* dest)
{
	for (int32_t i = 0; i < length; ++i)
	{
		if (Vector4::loadAligned(&dest[i * 4]) != source[i])
		{
			for (; i < length; ++i)
				source[i].storeAligned(&dest[i * 4]);
			return true;
		}
	}
	return false;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramVk", ProgramVk, IProgram)

ProgramVk::ProgramVk(Context* context, uint32_t& instances)
:	m_context(context)
,	m_instances(instances)
{
	Atomic::increment((int32_t&)m_instances);
	m_context->addCleanupListener(this);
}

ProgramVk::~ProgramVk()
{
	destroy();
	m_context->removeCleanupListener(this);
	Atomic::decrement((int32_t&)m_instances);
}

bool ProgramVk::create(
	ShaderModuleCache* shaderModuleCache,
	PipelineLayoutCache* pipelineLayoutCache,
	const ProgramResourceVk* resource,
	int32_t maxAnistropy,
	float mipBias,
	const wchar_t* const tag
)
{
	VkShaderStageFlags stageFlags;

	m_tag = tag;
	m_renderState = resource->m_renderState;
	m_shaderHash = resource->m_shaderHash;
	m_useTargetSize = resource->m_useTargetSize;
	m_localWorkGroupSize[0] = resource->m_localWorkGroupSize[0];
	m_localWorkGroupSize[1] = resource->m_localWorkGroupSize[1];
	m_localWorkGroupSize[2] = resource->m_localWorkGroupSize[2];

	// Get shader modules.
	if (!resource->m_vertexShader.empty() && !resource->m_fragmentShader.empty())
	{
		T_FATAL_ASSERT(resource->m_computeShader.empty());

		if ((m_vertexShaderModule = shaderModuleCache->get(resource->m_vertexShader, resource->m_vertexShaderHash)) == 0)
			return false;
		if ((m_fragmentShaderModule = shaderModuleCache->get(resource->m_fragmentShader, resource->m_fragmentShaderHash)) == 0)
			return false;

		stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;
	}
	else if (!resource->m_computeShader.empty())
	{
		if ((m_computeShaderModule = shaderModuleCache->get(resource->m_computeShader, resource->m_computeShaderHash)) == 0)
			return false;

		stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	}
	else
		return false;

	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(m_context->getPhysicalDevice(), &deviceProperties);
	uint32_t uniformBufferOffsetAlignment = std::max< uint32_t >(
		(uint32_t)deviceProperties.limits.minUniformBufferOffsetAlignment,
		sizeof(intptr_t)
	);

	// Create descriptor set layouts for shader uniforms.
	AlignedVector< VkDescriptorSetLayoutBinding  > dslb;

	// Each program has 3 uniform buffer bindings (Once, Frame and Draw).
	for (uint32_t i = 0; i < 3; ++i)
	{
		if (resource->m_uniformBufferSizes[i] == 0)
			continue;

		dslb.push_back({
			.binding = i + 2,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
			.descriptorCount = 1,
			.stageFlags = stageFlags
		});
	}

	// Append sampler bindings.
	for (const auto& sampler : resource->m_samplers)
	{
		if (sampler.binding < 0)
			continue;

		dslb.push_back({
			.binding = (uint32_t)sampler.binding,
			.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
			.descriptorCount = 1,
			.stageFlags = getShaderStageFlags(sampler.stages)
		});
	}

	// Append texture bindings.
	for (const auto& texture : resource->m_textures)
	{
		if (texture.binding < 0)
			continue;

		dslb.push_back({
			.binding = (uint32_t)texture.binding,
			.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			.descriptorCount = 1,
			.stageFlags = getShaderStageFlags(texture.stages)
		});
	}

	// Append image bindings.
	for (const auto& image : resource->m_images)
	{
		if (image.binding < 0)
			 continue;

		dslb.push_back({
			.binding = (uint32_t)image.binding,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			.descriptorCount = 1,
			.stageFlags = getShaderStageFlags(image.stages)
		});
	}

	// Append sbuffer bindings.
	for (const auto& sbuffer : resource->m_sbuffers)
	{
		if (sbuffer.binding < 0)
			continue;

		dslb.push_back({
			.binding = (uint32_t)sbuffer.binding,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
			.descriptorCount = 1,
			.stageFlags = getShaderStageFlags(sbuffer.stages)
		});
	}

	VkDescriptorSetLayoutCreateInfo dlci = {};
	dlci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	dlci.pNext = nullptr;
	dlci.bindingCount = (uint32_t)dslb.size();
	dlci.pBindings = dslb.c_ptr();

	if (!pipelineLayoutCache->get(
		resource->m_layoutHash,
		resource->m_useTargetSize,
		dlci,
		m_descriptorSetLayout,
		m_pipelineLayout
	))
		return false;

	// Create uniform buffer CPU side shadow data.
	for (uint32_t i = 0; i < 3; ++i)
	{
		m_uniformBuffers[i].size = resource->m_uniformBufferSizes[i] * sizeof(float);
		if (m_uniformBuffers[i].size == 0)
			continue;

		m_uniformBuffers[i].alignedSize = alignUp(m_uniformBuffers[i].size, uniformBufferOffsetAlignment);
		m_uniformBuffers[i].data.resize(resource->m_uniformBufferSizes[i], 0.0f);
	}

	// Create samplers.
	for (const auto& resourceSampler : resource->m_samplers)
	{
		VkSamplerCreateInfo sci = {};
		sci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sci.magFilter = c_filters[(int)resourceSampler.state.magFilter];
		sci.minFilter = c_filters[(int)resourceSampler.state.minFilter];
		sci.mipmapMode = c_mipMapModes[(int)resourceSampler.state.mipFilter];
		sci.addressModeU = c_addressModes[(int)resourceSampler.state.addressU];
		sci.addressModeV = c_addressModes[(int)resourceSampler.state.addressV];
		sci.addressModeW = c_addressModes[(int)resourceSampler.state.addressW];
		sci.mipLodBias = !resourceSampler.state.ignoreMips ? resourceSampler.state.mipBias + mipBias : 0.0f;
		
		if (maxAnistropy > 0)
			sci.anisotropyEnable = resourceSampler.state.useAnisotropic ? VK_TRUE : VK_FALSE;
		else
			sci.anisotropyEnable = VK_FALSE;

		sci.maxAnisotropy = (float)maxAnistropy;
		sci.compareEnable = (resourceSampler.state.compare != CompareFunction::None) ? VK_TRUE : VK_FALSE;
		sci.compareOp = c_compareOperations[(int)resourceSampler.state.compare];
		sci.minLod = 0.0f;
		sci.maxLod = std::numeric_limits< float >::max();
		sci.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		sci.unnormalizedCoordinates = VK_FALSE;

		VkSampler sampler = pipelineLayoutCache->getSampler(sci);
		if (!sampler)
			return false;

		m_samplers.push_back({ resourceSampler.binding, sampler });
	}

	// Create textures.
	m_textures.reserve(resource->m_textures.size());
	for (const auto& resourceTexture : resource->m_textures)
		m_textures.push_back({ resourceTexture.name + L" (" + getShaderStageNames(resourceTexture.stages) + L")", resourceTexture.binding });

	// Create images.
	m_images.reserve(resource->m_images.size());
	for (const auto& resourceImage : resource->m_images)
		m_images.push_back({ resourceImage.name + L" (" + getShaderStageNames(resourceImage.stages) + L")", resourceImage.binding });
	
	// Create sbuffers.
	m_sbuffers.reserve(resource->m_sbuffers.size());
	for (const auto& resourceSBuffer : resource->m_sbuffers)
		m_sbuffers.push_back({ resourceSBuffer.name + L" (" + getShaderStageNames(resourceSBuffer.stages) + L")", resourceSBuffer.binding });

	// Setup parameter mapping.
	for (auto p : resource->m_parameters)
	{
		auto& pm = m_parameterMap[getParameterHandle(p.name)];
		pm.name = p.name;
		pm.ubuffer = p.ubuffer;
		pm.ubufferOffset = p.ubufferOffset;
		pm.ubufferSize = p.ubufferSize;
		pm.textureIndex = p.textureIndex;
		pm.imageIndex = p.imageIndex;
		pm.sbufferIndex = p.sbufferIndex;
	}

	return true;
}

bool ProgramVk::validate(
	CommandBuffer* commandBuffer,
	VkPipelineBindPoint bindPoint,
	const float* targetSize
)
{
	// Set implicit parameters.
	if (m_useTargetSize)
	{
		const float value[4] = { targetSize[0], targetSize[1], 0.0f, 0.0f };
		vkCmdPushConstants(
			*commandBuffer,
			m_pipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			4 * sizeof(float),
			value
		);
	}

	// Set bindless resource indices.
	for (auto it : m_parameterMap)
	{
		const ParameterMap& pm = it.second;
		if (pm.ubuffer < 0)
			continue;

		if (pm.textureIndex >= 0)
		{
			ITexture* texture = m_textures[pm.textureIndex].texture;
			if (!texture)
			{
				log::error << L"No bound texture for parameter \"" << pm.name << L"\"." << Endl;
				return false;
			}

			ITexture* resolved = texture->resolve();
			T_ASSERT(resolved);

			uint32_t resourceIndex = ~0U;
			if (is_a< TextureVk >(resolved))
				resourceIndex = static_cast< TextureVk* >(resolved)->getImage()->getSampledResourceIndex();
			else if (is_a< RenderTargetVk >(resolved))
				resourceIndex = static_cast< RenderTargetVk* >(resolved)->getImageResolved()->getSampledResourceIndex();
			else if (is_a< RenderTargetDepthVk >(resolved))
				resourceIndex = static_cast< RenderTargetDepthVk* >(resolved)->getImage()->getSampledResourceIndex();

			T_FATAL_ASSERT(resourceIndex != ~0U);

			auto& ub = m_uniformBuffers[pm.ubuffer];
			if (storeIfNotEqual((const float*)&resourceIndex, 1, &ub.data[pm.ubufferOffset]))
				ub.dirty = true;
		}
		else if (pm.imageIndex >= 0)
		{
			ITexture* texture = m_images[pm.imageIndex].texture;
			if (!texture)
			{
				log::error << L"No bound texture for parameter \"" << pm.name << L"\"." << Endl;
				return false;
			}

			const int32_t mip = m_images[pm.imageIndex].mip;

			ITexture* resolved = texture->resolve();
			T_ASSERT(resolved);

			uint32_t resourceIndex = ~0U;
			if (is_a< TextureVk >(resolved))
				resourceIndex = static_cast< TextureVk* >(resolved)->getImage()->getStorageResourceIndex(mip);
			else if (is_a< RenderTargetVk >(resolved))
				resourceIndex = static_cast< RenderTargetVk* >(resolved)->getImageResolved()->getStorageResourceIndex(mip);
			else if (is_a< RenderTargetDepthVk >(resolved))
				resourceIndex = static_cast< RenderTargetDepthVk* >(resolved)->getImage()->getStorageResourceIndex(mip);

			T_FATAL_ASSERT(resourceIndex != ~0U);

			auto& ub = m_uniformBuffers[pm.ubuffer];
			if (storeIfNotEqual((const float*)&resourceIndex, 1, &ub.data[pm.ubufferOffset]))
				ub.dirty = true;			
		}
	}

	// Update content of uniform buffers.
	for (uint32_t i = 0; i < 3; ++i)
	{
		if (!m_uniformBuffers[i].size || !m_uniformBuffers[i].dirty)
			continue;

		auto pool = m_context->getUniformBufferPool(i);

		if (m_uniformBuffers[i].range.ptr)
			pool->free(m_uniformBuffers[i].range);

		if (!pool->allocate(m_uniformBuffers[i].alignedSize, m_uniformBuffers[i].range))
		{
			log::error << L"Out of uniform buffer pool memory (" << m_tag << L")!" << Endl;
			return false;
		}

		std::memcpy(
			m_uniformBuffers[i].range.ptr,
			m_uniformBuffers[i].data.c_ptr(),
			m_uniformBuffers[i].size
		);

		m_uniformBuffers[i].dirty = false;
	}

	if (!validateDescriptorSet())
		return false;

	// Get offsets into buffers.
	StaticVector< uint32_t, 3+32 > bufferOffsets;
	for (uint32_t i = 0; i < 3; ++i)
	{
		if (!m_uniformBuffers[i].size)
			continue;
		bufferOffsets.push_back(m_uniformBuffers[i].range.offset);
	}
	for (const auto& sbuffer : m_sbuffers)
	{
		if (!sbuffer.bufferView)
			continue;
		auto bvvk = sbuffer.bufferView;
		bufferOffsets.push_back(bvvk->getVkBufferOffset());
	}

	// Must match order defined in GlslResource.h
	const VkDescriptorSet descriptorSets[] =
	{
		m_descriptorSet,
		m_context->getBindlessTexturesDescriptorSet(),
		m_context->getBindlessImagesDescriptorSet()
	};

	vkCmdBindDescriptorSets(
		*commandBuffer,
		bindPoint,
		m_pipelineLayout,
		0,
		sizeof_array(descriptorSets), descriptorSets,
		(uint32_t)bufferOffsets.size(), bufferOffsets.c_ptr()
	);

	if (bindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS && m_renderState.stencilEnable)
		vkCmdSetStencilReference(
			*commandBuffer,
			VK_STENCIL_FRONT_AND_BACK,
			m_stencilReference
		);

	return true;
}

void ProgramVk::destroy()
{
	for (uint32_t i = 0; i < 3; ++i)
	{
		if (m_uniformBuffers[i].range.ptr)
			m_context->getUniformBufferPool(i)->free(m_uniformBuffers[i].range);
		m_uniformBuffers[i].range.ptr = nullptr;
	}

	for (auto it : m_descriptorSets)
	{
		m_context->addDeferredCleanup([
			descriptorSet = it.second
		](Context* cx) {
			vkFreeDescriptorSets(cx->getLogicalDevice(), cx->getDescriptorPool(), 1, &descriptorSet);
		});
	}
	m_descriptorSets.clear();

	m_vertexShaderModule = 0;
	m_fragmentShaderModule = 0;
	m_computeShaderModule = 0;

	m_descriptorSetLayout = 0;
	m_pipelineLayout = 0;

	m_samplers.clear();
	m_textures.clear();
	m_images.clear();
	m_sbuffers.clear();
}

void ProgramVk::setFloatParameter(handle_t handle, float param)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		auto& ub = m_uniformBuffers[i->second.ubuffer];
		if (storeIfNotEqual(&param, 1, &ub.data[i->second.ubufferOffset]))
			ub.dirty = true;
	}
}

void ProgramVk::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		auto& ub = m_uniformBuffers[i->second.ubuffer];
		if (storeIfNotEqual(param, length, &ub.data[i->second.ubufferOffset]))
			ub.dirty = true;
	}
}

void ProgramVk::setVectorParameter(handle_t handle, const Vector4& param)
{
	setVectorArrayParameter(handle, &param, 1);
}

void ProgramVk::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		T_FATAL_ASSERT (length * 4 <= (int)i->second.ubufferSize);
		auto& ub = m_uniformBuffers[i->second.ubuffer];
		if (storeIfNotEqual(param, length, &ub.data[i->second.ubufferOffset]))
			ub.dirty = true;
	}
}

void ProgramVk::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		auto& ub = m_uniformBuffers[i->second.ubuffer];
		param.storeAligned(&ub.data[i->second.ubufferOffset]);
		ub.dirty = true;
	}
}

void ProgramVk::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		T_FATAL_ASSERT (length * 16 <= (int)i->second.ubufferSize);
		auto& ub = m_uniformBuffers[i->second.ubuffer];
		for (int j = 0; j < length; ++j)
			param[j].storeAligned(&ub.data[i->second.ubufferOffset + j * 16]);
		ub.dirty = true;
	}
}

void ProgramVk::setTextureParameter(handle_t handle, ITexture* texture)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		T_FATAL_ASSERT(i->second.textureIndex >= 0);
		m_textures[i->second.textureIndex].texture = texture;
	}
}

void ProgramVk::setImageViewParameter(handle_t handle, ITexture* imageView, int mip)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		T_FATAL_ASSERT(i->second.imageIndex >= 0);
		m_images[i->second.imageIndex].texture = imageView;
		m_images[i->second.imageIndex].mip = mip;
	}
}

void ProgramVk::setBufferViewParameter(handle_t handle, const IBufferView* bufferView)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		T_FATAL_ASSERT(i->second.sbufferIndex >= 0);
		m_sbuffers[i->second.sbufferIndex].bufferView = (const BufferViewVk*)bufferView;
	}
}

void ProgramVk::setStencilReference(uint32_t stencilReference)
{
	m_stencilReference = stencilReference;
}

bool ProgramVk::validateDescriptorSet()
{
	DescriptorSetKey key;

	// Add uniform buffers to key.
	for (uint32_t i = 0; i < 3; ++i)
	{
		if (!m_uniformBuffers[i].size)
			continue;
		key.push_back((intptr_t)m_uniformBuffers[i].range.chain);
	}

	// Add non-bindless textures to key.
	for (const auto& texture : m_textures)
	{
		if (texture.binding < 0)
			continue;
		if (!texture.texture)
			return false;
		auto resolved = texture.texture->resolve();
		if (!resolved)
			return false;
		key.push_back((intptr_t)resolved);
	}

	// Add non-bindless images to key.
	for (const auto& image : m_images)
	{
		if (image.binding < 0)
			continue;
		if (!image.texture)
			return false;
		auto resolved = image.texture->resolve();
		if (!resolved)
			return false;
		key.push_back((intptr_t)resolved);
	}

	// Add sbuffers to key.
	for (const auto& sbuffer : m_sbuffers)
	{
		if (!sbuffer.bufferView)
			return false;
		key.push_back((intptr_t)sbuffer.bufferView->getVkBuffer());
	}

	// Get already created descriptor set for resources.
	for (const auto& k : m_descriptorSets)
	{
		if (k.first.size() == key.size())
		{
			if (memcmp(k.first.c_ptr(), key.c_ptr(), sizeof(intptr_t) * key.size()) == 0)
			{
				m_descriptorSet = k.second;
				return true;
			}
		}
	}

	// No such descriptor set found, need to create another set.
	VkDescriptorSetAllocateInfo dsai;
	dsai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	dsai.pNext = nullptr;
	dsai.descriptorPool = m_context->getDescriptorPool();
	dsai.descriptorSetCount = 1;
	dsai.pSetLayouts = &m_descriptorSetLayout;
	if (vkAllocateDescriptorSets(m_context->getLogicalDevice(), &dsai, &m_descriptorSet) != VK_SUCCESS)
	{
		log::error << L"Unable to allocate Vulkan descriptor set (" << m_tag << L")!" << Endl;
		m_descriptorSets.insert(key, 0);
		return false;
	}

	StaticVector< VkDescriptorBufferInfo, 16 > bufferInfos;
	StaticVector< VkDescriptorImageInfo, 32 + 32 > imageInfos;
	StaticVector< VkWriteDescriptorSet, 16 + 32 + 32 > writes;

	// Add uniform buffer bindings.
	for (uint32_t i = 0; i < 3; ++i)
	{
		if (!m_uniformBuffers[i].size)
			continue;

		auto& bufferInfo = bufferInfos.push_back({
			.buffer = *m_uniformBuffers[i].range.chain->getBuffer(),
			.range = m_uniformBuffers[i].size
		});

		writes.push_back({
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = m_descriptorSet,
			.dstBinding = i + 2,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
			.pBufferInfo = &bufferInfo
		});
	}

	// Add sampler bindings.
	for (const auto& sampler : m_samplers)
	{
		if (sampler.binding < 0)
			continue;

		auto& imageInfo = imageInfos.push_back({
			.sampler = sampler.sampler,
			.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED
		});

		writes.push_back({
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = m_descriptorSet,
			.dstBinding = (uint32_t)sampler.binding,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
			.pImageInfo = &imageInfo
		});
	}

	// Add non-bindless texture bindings.
	for (const auto& texture : m_textures)
	{
		if (texture.binding < 0)
			continue;

		T_ASSERT(texture.texture);
		auto resolved = texture.texture->resolve();
		T_ASSERT(resolved);

		auto& imageInfo = imageInfos.push_back();
		imageInfo.sampler = 0;

		if (is_a< TextureVk >(resolved))
		{
			imageInfo.imageView = static_cast< TextureVk* >(resolved)->getImage()->getVkImageView();
			imageInfo.imageLayout = static_cast< TextureVk* >(resolved)->getImage()->getVkImageLayout(0, 0);
		}
		else if (is_a< RenderTargetVk >(resolved))
		{
			imageInfo.imageView = static_cast< RenderTargetVk* >(resolved)->getImageResolved()->getVkImageView();
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
		else if (is_a< RenderTargetDepthVk >(resolved))
		{
			imageInfo.imageView = static_cast< RenderTargetDepthVk* >(resolved)->getImage()->getVkImageView();
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		}

		writes.push_back({
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = m_descriptorSet,
			.dstBinding = (uint32_t)texture.binding,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			.pImageInfo = &imageInfo
		});
	}

	// Add non-bindless image bindings.
	for (const auto& image : m_images)
	{
		if (image.binding < 0)
			continue;

		T_ASSERT(image.texture);
		auto resolved = image.texture->resolve();
		T_ASSERT(resolved);

		auto& imageInfo = imageInfos.push_back();
		imageInfo.sampler = 0;

		if (is_a< TextureVk >(resolved))
		{
			imageInfo.imageView = static_cast< TextureVk* >(resolved)->getImage()->getStorageVkImageView(image.mip);
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		}
		else if (is_a< RenderTargetVk >(resolved))
		{
			imageInfo.imageView = static_cast< RenderTargetVk* >(resolved)->getImageResolved()->getStorageVkImageView(image.mip);
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		}
		else if (is_a< RenderTargetDepthVk >(resolved))
		{
			imageInfo.imageView = static_cast< RenderTargetDepthVk* >(resolved)->getImage()->getStorageVkImageView(image.mip);
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		}

		writes.push_back({
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = m_descriptorSet,
			.dstBinding = (uint32_t)image.binding,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			.pImageInfo = &imageInfo
		});
	 }

	// Add sbuffer bindings.
	for (const auto& sbuffer : m_sbuffers)
	{
		if (sbuffer.binding < 0)
			continue;

		auto& bufferInfo = bufferInfos.push_back({
			.buffer = sbuffer.bufferView->getVkBuffer(),
			.range = sbuffer.bufferView->getVkBufferRange()
		});

		writes.push_back({
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = m_descriptorSet,
			.dstBinding = (uint32_t)sbuffer.binding,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
			.pBufferInfo = &bufferInfo
		});
	}

	vkUpdateDescriptorSets(
		m_context->getLogicalDevice(),
		(uint32_t)writes.size(),
		writes.c_ptr(),
		0,
		nullptr
	);

	m_descriptorSets.insert(key, m_descriptorSet);
	return true;
}

void ProgramVk::postCleanup()
{
	// Since some resource has been cleaned up we cannot guarantee integrity
	// of our cached descriptor sets, thus we need to rebuild every set.
	for (auto it : m_descriptorSets)
		vkFreeDescriptorSets(m_context->getLogicalDevice(), m_context->getDescriptorPool(), 1, &it.second);
	m_descriptorSets.reset();
}

bool ProgramVk::DescriptorSetKey::operator < (const DescriptorSetKey& rh) const
{
	if (size() < rh.size())
		return true;
	if (size() > rh.size())
		return false;

	for (uint32_t i = 0; i < size(); ++i)
	{
		if ((*this)[i] < rh[i])
			return true;
	}

	return false;
}

bool ProgramVk::DescriptorSetKey::operator > (const DescriptorSetKey& rh) const
{
	if (size() < rh.size())
		return false;
	if (size() > rh.size())
		return true;

	for (uint32_t i = 0; i < size(); ++i)
	{
		if ((*this)[i] > rh[i])
			return true;
	}

	return false;
}

}
