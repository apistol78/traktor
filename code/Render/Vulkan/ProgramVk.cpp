#include <cstring>
#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Matrix44.h"
#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/CubeTextureVk.h"
#include "Render/Vulkan/ProgramVk.h"
#include "Render/Vulkan/ProgramResourceVk.h"
#include "Render/Vulkan/RenderTargetDepthVk.h"
#include "Render/Vulkan/RenderTargetVk.h"
#include "Render/Vulkan/SimpleTextureVk.h"
#include "Render/Vulkan/StructBufferVk.h"
#include "Render/Vulkan/UniformBufferPoolVk.h"
#include "Render/Vulkan/UtilitiesVk.h"
#include "Render/Vulkan/VolumeTextureVk.h"

#undef max

namespace traktor
{
	namespace render
	{
		namespace
		{

handle_t s_handleTargetSize = 0;

bool storeIfNotEqual(const float* source, int length, float* dest)
{
	for (int i = 0; i < length; ++i)
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

bool storeIfNotEqual(const Vector4* source, int length, float* dest)
{
	for (int i = 0; i < length; ++i)
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

ProgramVk::ProgramVk(
	VkPhysicalDevice physicalDevice,
	VkDevice logicalDevice,
	VmaAllocator allocator,
	int32_t maxAnistropy
)
:	m_physicalDevice(physicalDevice)
,	m_logicalDevice(logicalDevice)
,	m_allocator(allocator)
,	m_maxAnistropy(maxAnistropy)
,	m_vertexShaderModule(0)
,	m_fragmentShaderModule(0)
,	m_computeShaderModule(0)
,	m_descriptorSetLayout(0)
,	m_pipelineLayout(0)
,	m_stencilReference(0)
,	m_hash(0)
{
	s_handleTargetSize = getParameterHandle(L"_vk_targetSize");
}

ProgramVk::~ProgramVk()
{
	destroy();
}

bool ProgramVk::create(const ProgramResourceVk* resource, const wchar_t* const tag)
{
	VkShaderStageFlags stageFlags;

#if defined(_DEBUG)
	m_tag = tag;
#endif

	m_renderState = resource->m_renderState;
	m_hash = resource->m_hash;

	if (!resource->m_vertexShader.empty() && !resource->m_fragmentShader.empty())
	{
		// Create vertex shader module.
		VkShaderModuleCreateInfo vsmci = {};
		vsmci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		vsmci.codeSize = resource->m_vertexShader.size() * sizeof(uint32_t);
		vsmci.pCode = &resource->m_vertexShader[0];
		if (vkCreateShaderModule(m_logicalDevice, &vsmci, nullptr, &m_vertexShaderModule) != VK_SUCCESS)
			return false;

		// Create fragment shader module.
		VkShaderModuleCreateInfo fsmci = {};
		fsmci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		fsmci.codeSize = resource->m_fragmentShader.size() * sizeof(uint32_t);
		fsmci.pCode = &resource->m_fragmentShader[0];
		if (vkCreateShaderModule(m_logicalDevice, &fsmci, nullptr, &m_fragmentShaderModule) != VK_SUCCESS)
			return false;

		stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;
	}
	else if (!resource->m_computeShader.empty())
	{
		// Create compute shader module.
		VkShaderModuleCreateInfo csmci = {};
		csmci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		csmci.codeSize = resource->m_computeShader.size() * sizeof(uint32_t);
		csmci.pCode = &resource->m_computeShader[0];
		if (vkCreateShaderModule(m_logicalDevice, &csmci, nullptr, &m_computeShaderModule) != VK_SUCCESS)
			return false;

		stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	}

	// Create descriptor set layouts for shader uniforms.
	AlignedVector< VkDescriptorSetLayoutBinding  > dslb;

	// Each program has 3 uniform buffer bindings (Once, Frame and Draw cbuffers).
	for (int32_t i = 0; i < 3; ++i)
	{
		auto& lb = dslb.push_back();
		lb = {};
		lb.binding = i;
		lb.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		lb.descriptorCount = 1;
		lb.stageFlags = stageFlags;
	}

	// Append sampler bindings.
	for (const auto& sampler : resource->m_samplers)
	{
		auto& lb = dslb.push_back();
		lb = {};
		lb.binding = sampler.binding;
		lb.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		lb.descriptorCount = 1;
		lb.stageFlags = stageFlags;
	}

	// Append texture bindings.
	for (const auto& texture : resource->m_textures)
	{
		auto& lb = dslb.push_back();
		lb = {};
		lb.binding = texture.binding;
		lb.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		lb.descriptorCount = 1;
		lb.stageFlags = stageFlags;
	}

	// Append sbuffer bindings.
	for (const auto& sbuffer : resource->m_sbuffers)
	{
		auto& lb = dslb.push_back();
		lb = {};
		lb.binding = sbuffer.binding;
		lb.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		lb.descriptorCount = 1;
		lb.stageFlags = stageFlags;
	}

	VkDescriptorSetLayoutCreateInfo dlci = {};
	dlci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	dlci.pNext = nullptr;
	dlci.bindingCount = (uint32_t)dslb.size();
	dlci.pBindings = dslb.c_ptr();

	if (vkCreateDescriptorSetLayout(m_logicalDevice, &dlci, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
		return false;

	// Create pipeline layout.
	VkPipelineLayoutCreateInfo lci = {};
	lci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	lci.setLayoutCount = 1;
	lci.pSetLayouts = &m_descriptorSetLayout;
	lci.pushConstantRangeCount = 0;
	lci.pPushConstantRanges = nullptr;

	if (vkCreatePipelineLayout(m_logicalDevice, &lci, nullptr, &m_pipelineLayout) != VK_SUCCESS)
		return false;

	// Create uniform shadow buffers.
	for (uint32_t i = 0; i < 3; ++i)
	{
		m_uniformBuffers[i].size = resource->m_uniformBufferSizes[i] * 4;
		m_uniformBuffers[i].data.resize(resource->m_uniformBufferSizes[i], 0.0f);
	}

	// Create samplers.
	for (const auto& resourceSampler : resource->m_samplers)
	{
		VkSampler sampler = 0;

		VkSamplerCreateInfo sci = {};
		sci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sci.magFilter = c_filters[resourceSampler.state.magFilter];
		sci.minFilter = c_filters[resourceSampler.state.minFilter];
		sci.mipmapMode = c_mipMapModes[resourceSampler.state.mipFilter];
		sci.addressModeU = c_addressModes[resourceSampler.state.addressU];
		sci.addressModeV = c_addressModes[resourceSampler.state.addressV];
		sci.addressModeW = c_addressModes[resourceSampler.state.addressW];
		sci.mipLodBias = resourceSampler.state.mipBias;
		
		if (m_maxAnistropy > 0)
			sci.anisotropyEnable = resourceSampler.state.useAnisotropic ? VK_TRUE : VK_FALSE;
		else
			sci.anisotropyEnable = VK_FALSE;

		sci.maxAnisotropy = m_maxAnistropy;
		sci.compareEnable = (resourceSampler.state.compare != CfNone) ? VK_TRUE : VK_FALSE;
		sci.compareOp = c_compareOperations[resourceSampler.state.compare];
		sci.minLod = 0.0f;
		sci.maxLod = resourceSampler.state.ignoreMips ? 0.0f : std::numeric_limits< float >::max();
		sci.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		sci.unnormalizedCoordinates = VK_FALSE;

		if (vkCreateSampler(m_logicalDevice, &sci, nullptr, &sampler) != VK_SUCCESS)
			return false;

		setObjectDebugName(m_logicalDevice, L"Sampler", (uint64_t)sampler, VK_OBJECT_TYPE_SAMPLER);

		m_samplers.push_back({ resourceSampler.binding, sampler });
	}

	// Create textures.
	for (const auto& resourceTexture : resource->m_textures)
	{
#if !defined(_DEBUG)
		m_textures.push_back({ resourceTexture.binding });
#else
		m_textures.push_back({ resourceTexture.name, resourceTexture.binding });
#endif
	}

	// Create sbuffers.
	for (const auto& resourceSBuffer : resource->m_sbuffers)
	{
#if !defined(_DEBUG)
		m_sbuffers.push_back({ resourceSBuffer.binding });
#else
		m_sbuffers.push_back({ resourceSBuffer.name, resourceSBuffer.binding });
#endif
	}

	// Setup parameter mapping.
	for (auto p : resource->m_parameters)
	{
		auto& pm = m_parameterMap[getParameterHandle(p.name)];
#if defined(_DEBUG)
		pm.name = p.name;
#endif
		pm.buffer = p.buffer;
		pm.offset = p.offset;
		pm.size = p.size;
	}

	return true;
}

bool ProgramVk::validateGraphics(VkDescriptorPool descriptorPool, VkCommandBuffer commandBuffer, UniformBufferPoolVk* uniformBufferPool, float targetSize[2])
{
	// Set implicit parameters.
	setVectorParameter(
		s_handleTargetSize,
		Vector4(targetSize[0], targetSize[1], 0.0f, 0.0f)
	);

	// Allocate a descriptor set for parameters.
	VkDescriptorSet descriptorSet = 0;

	VkDescriptorSetAllocateInfo allocateInfo;
	allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocateInfo.pNext = nullptr;
	allocateInfo.descriptorPool = descriptorPool;
	allocateInfo.descriptorSetCount = 1;
	allocateInfo.pSetLayouts = &m_descriptorSetLayout;

	if (vkAllocateDescriptorSets(m_logicalDevice, &allocateInfo, &descriptorSet) != VK_SUCCESS)
		return false;

#if defined(_DEBUG)
	// Set debug name of descriptor set.
	setObjectDebugName(m_logicalDevice, m_tag.c_str(), (uint64_t)descriptorSet, VK_OBJECT_TYPE_DESCRIPTOR_SET);
#endif

	AlignedVector< VkDescriptorBufferInfo > bufferInfos;
	AlignedVector< VkDescriptorImageInfo > imageInfos;
	AlignedVector< VkWriteDescriptorSet > writes;
	
	// Update scalar uniform buffers.
	for (uint32_t i = 0; i < 3; ++i)
	{
		if (!m_uniformBuffers[i].size)
			continue;

		// Grab a device buffer; cycle buffers for each update call.
		if (m_uniformBuffers[i].dirty)
		{
			if (!uniformBufferPool->acquire(
				m_uniformBuffers[i].size,
				m_uniformBuffers[i].buffer,
				m_uniformBuffers[i].allocation
			))
				return false;

			uint8_t* ptr = nullptr;
			if (vmaMapMemory(m_allocator, m_uniformBuffers[i].allocation, (void**)&ptr) != VK_SUCCESS)
				return false;

			std::memcpy(
				ptr,
				m_uniformBuffers[i].data.c_ptr(),
				m_uniformBuffers[i].size
			);

			vmaUnmapMemory(m_allocator, m_uniformBuffers[i].allocation);
			m_uniformBuffers[i].dirty = false;
		}

		auto& bufferInfo = bufferInfos.push_back();
		bufferInfo.buffer = m_uniformBuffers[i].buffer;
		bufferInfo.offset = 0;
		bufferInfo.range = m_uniformBuffers[i].size;

		auto& write = writes.push_back();
		write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstSet = descriptorSet;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write.pBufferInfo = &bufferInfo;
		write.dstArrayElement = 0;
		write.dstBinding = i;
	}

	// Update sampler bindings.
	for (const auto& sampler : m_samplers)
	{
		auto& imageInfo = imageInfos.push_back();
		imageInfo.sampler = sampler.sampler;
		imageInfo.imageView = 0;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		auto& write = writes.push_back();
		write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstSet = descriptorSet;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		write.pImageInfo = &imageInfo;
		write.dstArrayElement = 0;
		write.dstBinding = sampler.binding;
	}

	// Update texture bindings.
	for (const auto& texture : m_textures)
	{
		if (!texture.texture)
			continue;
		
		Ref< ITexture > resolved = texture.texture->resolve();
		if (!resolved)
			continue;

		VkImageView imageView = 0;
		if (is_a< SimpleTextureVk >(resolved))
			imageView = static_cast< SimpleTextureVk* >(resolved.ptr())->getVkImageView();
		else if (is_a< CubeTextureVk >(resolved))
			imageView = static_cast< CubeTextureVk* >(resolved.ptr())->getVkImageView();
		else if (is_a< RenderTargetVk >(resolved))
			imageView = static_cast< RenderTargetVk* >(resolved.ptr())->getVkImageView();
		else if (is_a< RenderTargetDepthVk >(resolved))
			imageView = static_cast< RenderTargetDepthVk* >(resolved.ptr())->getVkImageView();
		else if (is_a< VolumeTextureVk >(resolved))
			imageView = static_cast< VolumeTextureVk* >(resolved.ptr())->getVkImageView();

		if (!imageView)
			continue;

		auto& imageInfo = imageInfos.push_back();
		imageInfo.sampler = 0;
		imageInfo.imageView = imageView;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		auto& write = writes.push_back();
		write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstSet = descriptorSet;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		write.pImageInfo = &imageInfo;
		write.dstArrayElement = 0;
		write.dstBinding = texture.binding;
	}

	// Update sbuffer bindings.
	for (const auto& sbuffer : m_sbuffers)
	{
		if (!sbuffer.sbuffer)
			continue;

		auto sbvk = static_cast< StructBufferVk* >(sbuffer.sbuffer.ptr());

		auto& bufferInfo = bufferInfos.push_back();
		bufferInfo.buffer = sbvk->getVkBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = sbvk->getBufferSize();

		auto& write = writes.push_back();
		write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstSet = descriptorSet;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		write.pBufferInfo = &bufferInfo;
		write.dstArrayElement = 0;
		write.dstBinding = sbuffer.binding;
	}

	if (!writes.empty())
		vkUpdateDescriptorSets(m_logicalDevice, (uint32_t)writes.size(), writes.c_ptr(), 0, nullptr);

	// Push command.
	vkCmdBindDescriptorSets(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_pipelineLayout,
		0,
		1, &descriptorSet,
		0, nullptr
	);

	if (m_renderState.stencilEnable)
		vkCmdSetStencilReference(
			commandBuffer,
			VK_STENCIL_FRONT_AND_BACK,
			m_stencilReference
		);

	return true;
}

bool ProgramVk::validateCompute(VkDescriptorPool descriptorPool, VkCommandBuffer commandBuffer, UniformBufferPoolVk* uniformBufferPool)
{
	// Allocate a descriptor set for parameters.
	VkDescriptorSet descriptorSet = 0;

	VkDescriptorSetAllocateInfo allocateInfo;
	allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocateInfo.pNext = nullptr;
	allocateInfo.descriptorPool = descriptorPool;
	allocateInfo.descriptorSetCount = 1;
	allocateInfo.pSetLayouts = &m_descriptorSetLayout;

	if (vkAllocateDescriptorSets(m_logicalDevice, &allocateInfo, &descriptorSet) != VK_SUCCESS)
		return false;

	AlignedVector< VkDescriptorBufferInfo > bufferInfos;
	AlignedVector< VkDescriptorImageInfo > imageInfos;
	AlignedVector< VkWriteDescriptorSet > writes;
	
	// Update scalar uniform buffers.
	for (uint32_t i = 0; i < 3; ++i)
	{
		if (!m_uniformBuffers[i].size)
			continue;

		// Grab a device buffer; cycle buffers for each update call.
		if (m_uniformBuffers[i].dirty)
		{
			if (!uniformBufferPool->acquire(
				m_uniformBuffers[i].size,
				m_uniformBuffers[i].buffer,
				m_uniformBuffers[i].allocation
			))
				return false;

			uint8_t* ptr = nullptr;
			if (vmaMapMemory(m_allocator, m_uniformBuffers[i].allocation, (void**)&ptr) != VK_SUCCESS)
				return false;

			std::memcpy(
				ptr,
				m_uniformBuffers[i].data.c_ptr(),
				m_uniformBuffers[i].size
			);

			vmaUnmapMemory(m_allocator, m_uniformBuffers[i].allocation);
			m_uniformBuffers[i].dirty = false;
		}

		auto& bufferInfo = bufferInfos.push_back();
		bufferInfo.buffer = m_uniformBuffers[i].buffer;
		bufferInfo.offset = 0;
		bufferInfo.range = m_uniformBuffers[i].size;

		auto& write = writes.push_back();
		write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstSet = descriptorSet;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write.pBufferInfo = &bufferInfo;
		write.dstArrayElement = 0;
		write.dstBinding = i;
	}

	// Update sampler bindings.
	for (const auto& sampler : m_samplers)
	{
		auto& imageInfo = imageInfos.push_back();
		imageInfo.sampler = sampler.sampler;
		imageInfo.imageView = 0;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		auto& write = writes.push_back();
		write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstSet = descriptorSet;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		write.pImageInfo = &imageInfo;
		write.dstArrayElement = 0;
		write.dstBinding = sampler.binding;
	}

	// Update texture bindings.
	for (const auto& texture : m_textures)
	{
		if (!texture.texture)
			continue;
		
		Ref< ITexture > resolved = texture.texture->resolve();
		if (!resolved)
			continue;

		VkImageView imageView = 0;
		if (is_a< SimpleTextureVk >(resolved))
			imageView = static_cast< SimpleTextureVk* >(resolved.ptr())->getVkImageView();
		else if (is_a< CubeTextureVk >(resolved))
			imageView = static_cast< CubeTextureVk* >(resolved.ptr())->getVkImageView();
		else if (is_a< RenderTargetVk >(resolved))
			imageView = static_cast< RenderTargetVk* >(resolved.ptr())->getVkImageView();
		else if (is_a< RenderTargetDepthVk >(resolved))
			imageView = static_cast< RenderTargetDepthVk* >(resolved.ptr())->getVkImageView();
		else if (is_a< VolumeTextureVk >(resolved))
			imageView = static_cast< VolumeTextureVk* >(resolved.ptr())->getVkImageView();

		if (!imageView)
			continue;

		auto& imageInfo = imageInfos.push_back();
		imageInfo.sampler = 0;
		imageInfo.imageView = imageView;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		auto& write = writes.push_back();
		write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstSet = descriptorSet;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		write.pImageInfo = &imageInfo;
		write.dstArrayElement = 0;
		write.dstBinding = texture.binding;
	}

	// Update sbuffer bindings.
	for (const auto& sbuffer : m_sbuffers)
	{
		if (!sbuffer.sbuffer)
			continue;

		auto sbvk = static_cast< StructBufferVk* >(sbuffer.sbuffer.ptr());

		auto& bufferInfo = bufferInfos.push_back();
		bufferInfo.buffer = sbvk->getVkBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = sbvk->getBufferSize();

		auto& write = writes.push_back();
		write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstSet = descriptorSet;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		write.pBufferInfo = &bufferInfo;
		write.dstArrayElement = 0;
		write.dstBinding = sbuffer.binding;
	}

	if (!writes.empty())
		vkUpdateDescriptorSets(m_logicalDevice, (uint32_t)writes.size(), writes.c_ptr(), 0, nullptr);

	// Push command.
	vkCmdBindDescriptorSets(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_COMPUTE,
		m_pipelineLayout,
		0,
		1, &descriptorSet,
		0, nullptr
	);

	return true;
}

void ProgramVk::destroy()
{
	if (m_vertexShaderModule != 0)
	{
		vkDestroyShaderModule(m_logicalDevice, m_vertexShaderModule, 0);
		m_vertexShaderModule = 0;
	}
	if (m_fragmentShaderModule != 0)
	{
		vkDestroyShaderModule(m_logicalDevice, m_fragmentShaderModule, 0);
		m_fragmentShaderModule = 0;
	}
	if (m_computeShaderModule != 0)
	{
		vkDestroyShaderModule(m_logicalDevice, m_computeShaderModule, 0);
		m_computeShaderModule = 0;
	}
	if (m_descriptorSetLayout != 0)
	{
		vkDestroyDescriptorSetLayout(m_logicalDevice, m_descriptorSetLayout, 0);
		m_descriptorSetLayout = 0;
	}
	if (m_pipelineLayout != 0)
	{
		vkDestroyPipelineLayout(m_logicalDevice, m_pipelineLayout, 0);
		m_pipelineLayout = 0;
	}
	for (auto& sampler : m_samplers)
		vkDestroySampler(m_logicalDevice, sampler.sampler, 0);
	m_samplers.clear();
}

void ProgramVk::setFloatParameter(handle_t handle, float param)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		auto& ub = m_uniformBuffers[i->second.buffer];
		if (storeIfNotEqual(&param, 1, &ub.data[i->second.offset]))
			ub.dirty = true;
	}
}

void ProgramVk::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		auto& ub = m_uniformBuffers[i->second.buffer];
		if (storeIfNotEqual(param, length, &ub.data[i->second.offset]))
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
		T_FATAL_ASSERT (length * 4 <= i->second.size);
		auto& ub = m_uniformBuffers[i->second.buffer];
		if (storeIfNotEqual(param, length, &ub.data[i->second.offset]))
			ub.dirty = true;
	}
}

void ProgramVk::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		auto& ub = m_uniformBuffers[i->second.buffer];
		param.storeAligned(&ub.data[i->second.offset]);
		ub.dirty = true;
	}
}

void ProgramVk::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		T_FATAL_ASSERT (length * 16 <= i->second.size);
		auto& ub = m_uniformBuffers[i->second.buffer];
		for (int j = 0; j < length; ++j)
			param[j].storeAligned(&ub.data[i->second.offset + j * 16]);
		ub.dirty = true;
	}
}

void ProgramVk::setTextureParameter(handle_t handle, ITexture* texture)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
		m_textures[i->second.offset].texture = texture;
}

void ProgramVk::setStructBufferParameter(handle_t handle, StructBuffer* structBuffer)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
		m_sbuffers[i->second.offset].sbuffer = structBuffer;
}

void ProgramVk::setStencilReference(uint32_t stencilReference)
{
	m_stencilReference = stencilReference;
}

	}
}
