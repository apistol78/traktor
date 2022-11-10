/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Vulkan/BufferViewVk.h"

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

}
