/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Thread/Atomic.h"
#include "Render/Vulkan/BufferVk.h"

namespace traktor::render
{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.BufferVk", BufferVk, Buffer)

BufferVk::BufferVk(Context* context, uint32_t bufferSize, uint32_t& instances)
:	Buffer(bufferSize)
,	m_context(context)
,	m_instances(instances)
{
	Atomic::increment((int32_t&)m_instances);
}

BufferVk::~BufferVk()
{
	Atomic::decrement((int32_t&)m_instances);
}

}
