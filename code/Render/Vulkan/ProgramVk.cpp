#include <cstring>
#include <limits>
#include "Core/Math/Matrix44.h"
#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/CubeTextureVk.h"
#include "Render/Vulkan/ProgramVk.h"
#include "Render/Vulkan/ProgramResourceVk.h"
#include "Render/Vulkan/RenderTargetDepthVk.h"
#include "Render/Vulkan/RenderTargetVk.h"
#include "Render/Vulkan/SimpleTextureVk.h"
#include "Render/Vulkan/StructBufferVk.h"
#include "Render/Vulkan/UtilitiesVk.h"
#include "Render/Vulkan/VolumeTextureVk.h"

#undef max

namespace traktor
{
	namespace render
	{
		namespace
		{

const uint32_t c_deviceBufferCount = 32;
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

ProgramVk::ProgramVk()
:	m_vertexShaderModule(0)
,	m_fragmentShaderModule(0)
,	m_computeShaderModule(0)
,	m_descriptorSetLayout(0)
,	m_pipelineLayout(0)
,	m_hash(0)
{
	s_handleTargetSize = getParameterHandle(L"_vk_targetSize");
}

ProgramVk::~ProgramVk()
{
	destroy();
}

bool ProgramVk::create(VkPhysicalDevice physicalDevice, VkDevice device, const ProgramResourceVk* resource)
{
	VkShaderStageFlags stageFlags;

	m_renderState = resource->m_renderState;
	m_hash = resource->m_hash;

	if (!resource->m_vertexShader.empty() && !resource->m_fragmentShader.empty())
	{
		// Create vertex shader module.
		VkShaderModuleCreateInfo vsmci = {};
		vsmci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		vsmci.codeSize = resource->m_vertexShader.size() * sizeof(uint32_t);
		vsmci.pCode = &resource->m_vertexShader[0];
		if (vkCreateShaderModule(device, &vsmci, nullptr, &m_vertexShaderModule) != VK_SUCCESS)
			return false;

		// Create fragment shader module.
		VkShaderModuleCreateInfo fsmci = {};
		fsmci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		fsmci.codeSize = resource->m_fragmentShader.size() * sizeof(uint32_t);
		fsmci.pCode = &resource->m_fragmentShader[0];
		if (vkCreateShaderModule(device, &fsmci, nullptr, &m_fragmentShaderModule) != VK_SUCCESS)
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
		if (vkCreateShaderModule(device, &csmci, nullptr, &m_computeShaderModule) != VK_SUCCESS)
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

	if (vkCreateDescriptorSetLayout(device, &dlci, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
		return false;

	// Create pipeline layout.
	VkPipelineLayoutCreateInfo lci = {};
	lci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	lci.setLayoutCount = 1;
	lci.pSetLayouts = &m_descriptorSetLayout;
	lci.pushConstantRangeCount = 0;
	lci.pPushConstantRanges = nullptr;

	if (vkCreatePipelineLayout(device, &lci, nullptr, &m_pipelineLayout) != VK_SUCCESS)
		return false;

	// Create uniform buffers.
	for (uint32_t i = 0; i < 3; ++i)
	{
		if (resource->m_uniformBufferSizes[i] > 0)
		{
			m_uniformBuffers[i].deviceBuffers.resize(c_deviceBufferCount);
			for (uint32_t j = 0; j < c_deviceBufferCount; ++j)
			{
				VkBufferCreateInfo bci = {};
				bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				bci.pNext = nullptr;
				bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
				bci.size = resource->m_uniformBufferSizes[i] * 4;
				bci.queueFamilyIndexCount = 0;
				bci.pQueueFamilyIndices = nullptr;
				bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
				bci.flags = 0;
				if (vkCreateBuffer(device, &bci, nullptr, &m_uniformBuffers[i].deviceBuffers[j].buffer) != VK_SUCCESS)
					return false;

				VkMemoryRequirements memoryRequirements = {};
				vkGetBufferMemoryRequirements(device, m_uniformBuffers[i].deviceBuffers[j].buffer, &memoryRequirements);

				VkMemoryAllocateInfo bai = {};
				bai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				bai.allocationSize = memoryRequirements.size;
				bai.memoryTypeIndex = getMemoryTypeIndex(physicalDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memoryRequirements);
				if (vkAllocateMemory(device, &bai, nullptr, &m_uniformBuffers[i].deviceBuffers[j].memory) != VK_SUCCESS)
					return false;

				vkBindBufferMemory(
					device,
					m_uniformBuffers[i].deviceBuffers[j].buffer,
					m_uniformBuffers[i].deviceBuffers[j].memory,
					0
				);
			}

			m_uniformBuffers[i].size = resource->m_uniformBufferSizes[i] * 4;
			m_uniformBuffers[i].data.resize(resource->m_uniformBufferSizes[i], 0.0f);
		}
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
		sci.anisotropyEnable = VK_FALSE; // resourceSampler.state.useAnisotropic ? VK_TRUE : VK_FALSE;
		sci.maxAnisotropy = 0.0f;
		sci.compareEnable = (resourceSampler.state.compare != CfNone) ? VK_TRUE : VK_FALSE;
		sci.compareOp = c_compareOperations[resourceSampler.state.compare];
		sci.minLod = 0.0f;
		sci.maxLod = resourceSampler.state.ignoreMips ? 0.0f : std::numeric_limits< float >::max();
		sci.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		sci.unnormalizedCoordinates = VK_FALSE;

		if (vkCreateSampler(device, &sci, nullptr, &sampler) != VK_SUCCESS)
			return false;

		m_samplers.push_back({ resourceSampler.binding, sampler });
	}

	// Create textures.
	for (const auto& resourceTexture : resource->m_textures)
	{
		m_textures.push_back({ resourceTexture.binding });
	}

	// Create sbuffers.
	for (const auto& resourceSBuffer : resource->m_sbuffers)
	{
		m_sbuffers.push_back({ resourceSBuffer.binding });
	}

	// Setup parameter mapping.
	for (auto p : resource->m_parameters)
	{
		auto& pm = m_parameterMap[getParameterHandle(p.name)];
		pm.buffer = p.buffer;
		pm.offset = p.offset;
		pm.size = p.size;
	}

	return true;
}

bool ProgramVk::validateGraphics(VkDevice device, VkDescriptorPool descriptorPool, VkCommandBuffer commandBuffer, float targetSize[2])
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

	if (vkAllocateDescriptorSets(device, &allocateInfo, &descriptorSet) != VK_SUCCESS)
		return false;

	AlignedVector< VkDescriptorBufferInfo > bufferInfos;
	AlignedVector< VkDescriptorImageInfo > imageInfos;
	AlignedVector< VkWriteDescriptorSet > writes;
	
	// Update scalar uniform buffers.
	for (uint32_t i = 0; i < 3; ++i)
	{
		if (!m_uniformBuffers[i].size)
			continue;

		// Grab a device buffer; cycle buffers for each draw call.
		DeviceBuffer& db = m_uniformBuffers[i].deviceBuffers[m_uniformBuffers[i].updateCount];
		m_uniformBuffers[i].updateCount = (m_uniformBuffers[i].updateCount + 1) % c_deviceBufferCount;

		uint8_t* ptr = nullptr;
		if (vkMapMemory(device, db.memory, 0, m_uniformBuffers[i].size, 0, (void **)&ptr) != VK_SUCCESS)
			return false;

		std::memcpy(
			ptr,
			m_uniformBuffers[i].data.c_ptr(),
			m_uniformBuffers[i].size
		);

		vkUnmapMemory(device, db.memory);

		auto& bufferInfo = bufferInfos.push_back();
		bufferInfo.buffer = db.buffer;
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
		vkUpdateDescriptorSets(device, (uint32_t)writes.size(), writes.c_ptr(), 0, nullptr);

	// Push command.
	vkCmdBindDescriptorSets(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_pipelineLayout,
		0,
		1, &descriptorSet,
		0, nullptr
	);

	return true;
}

bool ProgramVk::validateCompute(VkDevice device, VkDescriptorPool descriptorPool, VkCommandBuffer commandBuffer)
{
	// Allocate a descriptor set for parameters.
	VkDescriptorSet descriptorSet = 0;

	VkDescriptorSetAllocateInfo allocateInfo;
	allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocateInfo.pNext = nullptr;
	allocateInfo.descriptorPool = descriptorPool;
	allocateInfo.descriptorSetCount = 1;
	allocateInfo.pSetLayouts = &m_descriptorSetLayout;

	if (vkAllocateDescriptorSets(device, &allocateInfo, &descriptorSet) != VK_SUCCESS)
		return false;

	AlignedVector< VkDescriptorBufferInfo > bufferInfos;
	AlignedVector< VkDescriptorImageInfo > imageInfos;
	AlignedVector< VkWriteDescriptorSet > writes;
	
	// Update scalar uniform buffers.
	for (uint32_t i = 0; i < 3; ++i)
	{
		if (!m_uniformBuffers[i].size)
			continue;

		// Grab a device buffer; cycle buffers for each draw call.
		DeviceBuffer& db = m_uniformBuffers[i].deviceBuffers[m_uniformBuffers[i].updateCount];
		m_uniformBuffers[i].updateCount = (m_uniformBuffers[i].updateCount + 1) % c_deviceBufferCount;

		uint8_t* ptr = nullptr;
		if (vkMapMemory(device, db.memory, 0, m_uniformBuffers[i].size, 0, (void **)&ptr) != VK_SUCCESS)
			return false;

		std::memcpy(
			ptr,
			m_uniformBuffers[i].data.c_ptr(),
			m_uniformBuffers[i].size
		);

		vkUnmapMemory(device, db.memory);

		auto& bufferInfo = bufferInfos.push_back();
		bufferInfo.buffer = db.buffer;
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
		vkUpdateDescriptorSets(device, (uint32_t)writes.size(), writes.c_ptr(), 0, nullptr);

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
}

void ProgramVk::setFloatParameter(handle_t handle, float param)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		auto& ub = m_uniformBuffers[i->second.buffer];
		if (storeIfNotEqual(&param, 1, &ub.data[i->second.offset]))
		{
			//if (i->second.cbuffer[0])
			//	i->second.cbuffer[0]->dirty = true;
			//if (i->second.cbuffer[1])
			//	i->second.cbuffer[1]->dirty = true;
		}
	}
}

void ProgramVk::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		auto& ub = m_uniformBuffers[i->second.buffer];
		float* out = &ub.data[i->second.offset];
		for (int j = 0; j < length - 1; ++j)
		{
			Vector4(Scalar(param[j])).storeAligned(out);
			out += 4;
		}
		*out++ = param[length - 1];

		//if (i->second.cbuffer[0])
		//	i->second.cbuffer[0]->dirty = true;
		//if (i->second.cbuffer[1])
		//	i->second.cbuffer[1]->dirty = true;
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
		{
			//if (i->second.cbuffer[0])
			//	i->second.cbuffer[0]->dirty = true;
			//if (i->second.cbuffer[1])
			//	i->second.cbuffer[1]->dirty = true;
		}
	}
}

void ProgramVk::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		auto& ub = m_uniformBuffers[i->second.buffer];
		param.storeAligned(&ub.data[i->second.offset]);
		//if (i->second.cbuffer[0])
		//	i->second.cbuffer[0]->dirty = true;
		//if (i->second.cbuffer[1])
		//	i->second.cbuffer[1]->dirty = true;
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
		//if (i->second.cbuffer[0])
		//	i->second.cbuffer[0]->dirty = true;
		//if (i->second.cbuffer[1])
		//	i->second.cbuffer[1]->dirty = true;
	}
}

void ProgramVk::setTextureParameter(handle_t handle, ITexture* texture)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		m_textures[i->second.offset].texture = texture;
	}
}

void ProgramVk::setStructBufferParameter(handle_t handle, StructBuffer* structBuffer)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		m_sbuffers[i->second.offset].sbuffer = structBuffer;
	}
}

void ProgramVk::setStencilReference(uint32_t stencilReference)
{
}

	}
}
