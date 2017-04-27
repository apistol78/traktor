/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstring>
#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/ProgramVk.h"
#include "Render/Vulkan/ProgramResourceVk.h"
#include "Render/Vulkan/UtilitiesVk.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

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
{
}

ProgramVk::~ProgramVk()
{
	destroy();
}

bool ProgramVk::create(VkPhysicalDevice physicalDevice, VkDevice device, const ProgramResourceVk* resource)
{
	m_renderState = resource->m_renderState;

	// Create vertex shader module.
	VkShaderModuleCreateInfo vertexShaderCreationInfo = {};
	vertexShaderCreationInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vertexShaderCreationInfo.codeSize = resource->m_vertexShader.size() * sizeof(uint32_t);
	vertexShaderCreationInfo.pCode = &resource->m_vertexShader[0];
	if (vkCreateShaderModule(device, &vertexShaderCreationInfo, nullptr, &m_vertexShaderModule) != VK_SUCCESS)
		return false;

	// Create fragment shader module.
	VkShaderModuleCreateInfo fragmentShaderCreationInfo = {};
	fragmentShaderCreationInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	fragmentShaderCreationInfo.codeSize = resource->m_fragmentShader.size() * sizeof(uint32_t);
	fragmentShaderCreationInfo.pCode = &resource->m_fragmentShader[0];
	if (vkCreateShaderModule(device, &fragmentShaderCreationInfo, nullptr, &m_fragmentShaderModule) != VK_SUCCESS)
		return false;

	// Create uniform buffers.
	for (uint32_t i = 0; i < 3; ++i)
	{
		// Vertex
		if (resource->m_vertexUniformBuffers[i].size > 0)
		{
			m_vertexUniformBuffers[i].deviceBuffers.resize(4);
			for (uint32_t j = 0; j < 4; ++j)
			{
				VkBufferCreateInfo uniformBufferCreationInfo = {};
				uniformBufferCreationInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				uniformBufferCreationInfo.pNext = nullptr;
				uniformBufferCreationInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
				uniformBufferCreationInfo.size = resource->m_vertexUniformBuffers[i].size * 4;
				uniformBufferCreationInfo.queueFamilyIndexCount = 0;
				uniformBufferCreationInfo.pQueueFamilyIndices = nullptr;
				uniformBufferCreationInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
				uniformBufferCreationInfo.flags = 0;
				if (vkCreateBuffer(device, &uniformBufferCreationInfo, nullptr, &m_vertexUniformBuffers[i].deviceBuffers[j].buffer) != VK_SUCCESS)
					return false;

				VkMemoryRequirements memoryRequirements = {};
				vkGetBufferMemoryRequirements(device, m_vertexUniformBuffers[i].deviceBuffers[j].buffer, &memoryRequirements);

				VkMemoryAllocateInfo bufferAllocateInfo = {};
				bufferAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				bufferAllocateInfo.allocationSize = memoryRequirements.size;
				bufferAllocateInfo.memoryTypeIndex = getMemoryTypeIndex(physicalDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memoryRequirements);
				if (vkAllocateMemory(device, &bufferAllocateInfo, nullptr, &m_vertexUniformBuffers[i].deviceBuffers[j].memory) != VK_SUCCESS)
					return false;

				vkBindBufferMemory(device, m_vertexUniformBuffers[i].deviceBuffers[j].buffer, m_vertexUniformBuffers[i].deviceBuffers[j].memory, 0);
			}

			m_vertexUniformBuffers[i].size = resource->m_vertexUniformBuffers[i].size * 4;

			for (auto p : resource->m_vertexUniformBuffers[i].parameters)
			{
				ParameterMap pm;
				pm.offset = p.offset;
				pm.size = p.size;
				pm.uniformBufferOffset = p.uniformBufferOffset;
				m_vertexUniformBuffers[i].parameters.push_back(pm);
			}
		}

		// Fragment
		if (resource->m_fragmentUniformBuffers[i].size > 0)
		{
			m_fragmentUniformBuffers[i].deviceBuffers.resize(4);
			for (uint32_t j = 0; j < 4; ++j)
			{
				VkBufferCreateInfo uniformBufferCreationInfo = {};
				uniformBufferCreationInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				uniformBufferCreationInfo.pNext = nullptr;
				uniformBufferCreationInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
				uniformBufferCreationInfo.size = resource->m_fragmentUniformBuffers[i].size * 4;
				uniformBufferCreationInfo.queueFamilyIndexCount = 0;
				uniformBufferCreationInfo.pQueueFamilyIndices = nullptr;
				uniformBufferCreationInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
				uniformBufferCreationInfo.flags = 0;
				if (vkCreateBuffer(device, &uniformBufferCreationInfo, nullptr, &m_fragmentUniformBuffers[i].deviceBuffers[j].buffer) != VK_SUCCESS)
					return false;

				VkMemoryRequirements memoryRequirements = {};
				vkGetBufferMemoryRequirements(device, m_fragmentUniformBuffers[i].deviceBuffers[j].buffer, &memoryRequirements);

				VkMemoryAllocateInfo bufferAllocateInfo = {};
				bufferAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				bufferAllocateInfo.allocationSize = memoryRequirements.size;
				bufferAllocateInfo.memoryTypeIndex = getMemoryTypeIndex(physicalDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memoryRequirements);
				if (vkAllocateMemory(device, &bufferAllocateInfo, nullptr, &m_fragmentUniformBuffers[i].deviceBuffers[j].memory) != VK_SUCCESS)
					return false;

				vkBindBufferMemory(device, m_fragmentUniformBuffers[i].deviceBuffers[j].buffer, m_fragmentUniformBuffers[i].deviceBuffers[j].memory, 0);
			}

			m_fragmentUniformBuffers[i].size = resource->m_fragmentUniformBuffers[i].size * 4;

			for (auto p : resource->m_fragmentUniformBuffers[i].parameters)
			{
				ParameterMap pm;
				pm.offset = p.offset;
				pm.size = p.size;
				pm.uniformBufferOffset = p.uniformBufferOffset;
				m_fragmentUniformBuffers[i].parameters.push_back(pm);
			}
		}
	}

	// Setup program's parameter data.
	m_parameterScalarData.resize(resource->m_parameterScalarSize, 0.0f);
	for (auto pd : resource->m_parameters)
	{
		ParameterMap& pm = m_parameterMap[getParameterHandle(pd.name)];
		pm.offset = pd.offset;
		pm.size = pd.size;
	}

	return true;
}

bool ProgramVk::validate(VkDevice device, VkDescriptorSet descriptorSet)
{
	for (uint32_t i = 0; i < 3; ++i)
	{
		if (!m_vertexUniformBuffers[i].size)
			continue;

		DeviceBuffer& db = m_vertexUniformBuffers[i].deviceBuffers[m_vertexUniformBuffers[i].updateCount];
		m_vertexUniformBuffers[i].updateCount = (m_vertexUniformBuffers[i].updateCount + 1) % 4;

		uint8_t* ptr = 0;
		if (vkMapMemory(device, db.memory, 0, m_vertexUniformBuffers[i].size, 0, (void **)&ptr) != VK_SUCCESS)
			return false;

		for (auto p : m_vertexUniformBuffers[i].parameters)
		{
			std::memcpy(
				ptr + p.uniformBufferOffset * 4,
				&m_parameterScalarData[p.offset],
				p.size * sizeof(float)
			);
		}

		vkUnmapMemory(device, db.memory);

		VkDescriptorBufferInfo bufferInfo;
		bufferInfo.buffer = db.buffer;
		bufferInfo.offset = 0;
		bufferInfo.range = m_vertexUniformBuffers[i].size;

		VkWriteDescriptorSet writes[1];
		writes[0] = {};
		writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[0].pNext = nullptr;
		writes[0].dstSet = descriptorSet;
		writes[0].descriptorCount = 1;
		writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writes[0].pBufferInfo = &bufferInfo;
		writes[0].dstArrayElement = 0;
		writes[0].dstBinding = i;
		vkUpdateDescriptorSets(device, 1, writes, 0, nullptr);
	}

	for (uint32_t i = 0; i < 3; ++i)
	{
		if (!m_fragmentUniformBuffers[i].size)
			continue;

		DeviceBuffer& db = m_fragmentUniformBuffers[i].deviceBuffers[m_fragmentUniformBuffers[i].updateCount];
		m_fragmentUniformBuffers[i].updateCount = (m_fragmentUniformBuffers[i].updateCount + 1) % 4;

		uint8_t* ptr = 0;
		if (vkMapMemory(device, db.memory, 0, m_fragmentUniformBuffers[i].size, 0, (void **)&ptr) != VK_SUCCESS)
			return false;

		for (auto p : m_fragmentUniformBuffers[i].parameters)
		{
			std::memcpy(
				ptr + p.uniformBufferOffset * 4,
				&m_parameterScalarData[p.offset],
				p.size * sizeof(float)
			);
		}

		vkUnmapMemory(device, db.memory);

		VkDescriptorBufferInfo bufferInfo;
		bufferInfo.buffer = db.buffer;
		bufferInfo.offset = 0;
		bufferInfo.range = m_fragmentUniformBuffers[i].size;

		VkWriteDescriptorSet writes[1];
		writes[0] = {};
		writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[0].pNext = nullptr;
		writes[0].dstSet = descriptorSet;
		writes[0].descriptorCount = 1;
		writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writes[0].pBufferInfo = &bufferInfo;
		writes[0].dstArrayElement = 0;
		writes[0].dstBinding = i + 3;
		vkUpdateDescriptorSets(device, 1, writes, 0, nullptr);
	}

	return true;
}

void ProgramVk::destroy()
{
}

void ProgramVk::setFloatParameter(handle_t handle, float param)
{
	SmallMap< handle_t, ParameterMap >::iterator i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		if (storeIfNotEqual(&param, 1, &m_parameterScalarData[i->second.offset]))
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
	SmallMap< handle_t, ParameterMap >::iterator i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		float* out = &m_parameterScalarData[i->second.offset];
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
	SmallMap< handle_t, ParameterMap >::iterator i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		T_FATAL_ASSERT (length * 4 <= i->second.size);
		if (storeIfNotEqual(param, length, &m_parameterScalarData[i->second.offset]))
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
	SmallMap< handle_t, ParameterMap >::iterator i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		param.storeAligned(&m_parameterScalarData[i->second.offset]);
		//if (i->second.cbuffer[0])
		//	i->second.cbuffer[0]->dirty = true;
		//if (i->second.cbuffer[1])
		//	i->second.cbuffer[1]->dirty = true;
	}
}

void ProgramVk::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	SmallMap< handle_t, ParameterMap >::iterator i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		T_FATAL_ASSERT (length * 16 <= i->second.size);
		for (int j = 0; j < length; ++j)
			param[j].storeAligned(&m_parameterScalarData[i->second.offset + j * 16]);
		//if (i->second.cbuffer[0])
		//	i->second.cbuffer[0]->dirty = true;
		//if (i->second.cbuffer[1])
		//	i->second.cbuffer[1]->dirty = true;
	}
}

void ProgramVk::setTextureParameter(handle_t handle, ITexture* texture)
{
}

void ProgramVk::setStencilReference(uint32_t stencilReference)
{
}

	}
}
