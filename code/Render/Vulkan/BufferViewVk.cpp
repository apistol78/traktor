/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Vulkan/BufferViewVk.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/Context.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.BufferViewVk", BufferViewVk, IBufferView)

BufferViewVk::BufferViewVk(VkBuffer buffer, uint32_t offset, uint32_t range, uint32_t size)
:	m_buffer(buffer)
,	m_offset(offset)
,	m_range(range)
,	m_size(size)
{
}

VkDeviceAddress BufferViewVk::getDeviceAddress(Context* context) const
{
	VkBufferDeviceAddressInfo deviceAddressInfo =
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.pNext = nullptr,
		.buffer = m_buffer
	};
	return vkGetBufferDeviceAddressKHR(context->getLogicalDevice(), &deviceAddressInfo) + m_offset;
}

}
