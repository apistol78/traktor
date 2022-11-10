/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Config.h"
#include "Render/Vulkan/Private/ApiBuffer.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/Context.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ApiBuffer", ApiBuffer, Object)

ApiBuffer::ApiBuffer(Context* context)
:	m_context(context)
{
}

ApiBuffer::~ApiBuffer()
{
	T_FATAL_ASSERT_M(m_context == nullptr, L"Buffer not properly destroyed.");
}

bool ApiBuffer::create(uint32_t bufferSize, uint32_t usageBits, bool cpuAccess, bool gpuAccess)
{
	T_FATAL_ASSERT(m_buffer == 0);
	T_FATAL_ASSERT(bufferSize > 0);

	VkBufferCreateInfo bci = {};
	bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bci.size = bufferSize;
	bci.usage = usageBits;
	bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo aci = {};
    if (cpuAccess && gpuAccess)
	{
	    aci.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		aci.requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	}
    else if (!cpuAccess && gpuAccess)
        aci.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    else if (cpuAccess && !gpuAccess)
        aci.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    else
        return false;

	if (vmaCreateBuffer(m_context->getAllocator(), &bci, &aci, &m_buffer, &m_allocation, nullptr) != VK_SUCCESS)
		return false;

    return true;
}

void ApiBuffer::destroy()
{
	T_FATAL_ASSERT_M(m_locked == nullptr, L"Buffer still locked.");
	if (m_buffer != 0)
	{
		m_context->addDeferredCleanup([
			buffer = m_buffer,
			allocation = m_allocation
		](Context* cx) {
			vmaDestroyBuffer(cx->getAllocator(), buffer, allocation);
		});
	}
	m_allocation = 0;
	m_buffer = 0;
	m_context = nullptr;
}

void* ApiBuffer::lock()
{
	T_FATAL_ASSERT_M(m_locked == nullptr, L"Buffer already locked.");
	if (vmaMapMemory(m_context->getAllocator(), m_allocation, &m_locked) != VK_SUCCESS)
		m_locked = nullptr;
	return m_locked;
}

void ApiBuffer::unlock()
{
	T_FATAL_ASSERT_M(m_locked != nullptr, L"Buffer not locked.");
	vmaUnmapMemory(m_context->getAllocator(), m_allocation);
	m_locked = nullptr;
}

}