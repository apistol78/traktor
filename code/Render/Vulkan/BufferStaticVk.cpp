/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Render/Vulkan/BufferStaticVk.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Context.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.BufferStaticVk", BufferStaticVk, BufferVk)

BufferStaticVk::BufferStaticVk(Context* context, uint32_t bufferSize, uint32_t& instances)
:	BufferVk(context, bufferSize, instances)
{
}

BufferStaticVk::~BufferStaticVk()
{
	teardown();
}

bool BufferStaticVk::create(uint32_t usageBits)
{
	const uint32_t bufferSize = getBufferSize();
	if (!bufferSize)
		return false;

	m_buffer = new ApiBuffer(m_context);
	if (!m_buffer->create(bufferSize, usageBits | VK_BUFFER_USAGE_TRANSFER_DST_BIT, false, true))
		return false;

	m_bufferView = BufferViewVk(*m_buffer, 0, bufferSize, bufferSize);
	m_size = bufferSize;
	return true;
}

void BufferStaticVk::destroy()
{
	// Only relinquish ownership; the buffer view handed out to render blocks has
	// to stay valid until every context which might reference it has been
	// rendered. Teardown is performed by the destructor which runs once the
	// retirement fence has been passed. \sa ResourceMorgue
}

void BufferStaticVk::teardown()
{
	safeDestroy(m_buffer);
	safeDestroy(m_stageBuffer);
	m_context = nullptr;
}

void* BufferStaticVk::lock()
{
	T_FATAL_ASSERT(m_stageBuffer == nullptr);

	m_stageBuffer = new ApiBuffer(m_context);
	if (!m_stageBuffer->create(m_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true, true))
	{
		safeDestroy(m_stageBuffer);
		return nullptr;
	}

	return m_stageBuffer->lock();
}

void BufferStaticVk::unlock()
{
	m_stageBuffer->unlock();

	Ref< Buffer > self = this;
	Ref< ApiBuffer > stageBuffer = m_stageBuffer;
	m_stageBuffer = nullptr;

	m_context->addDeferredUpload(
		[self, stageBuffer, this](Context* cx, CommandBuffer* commandBuffer) mutable {
			if (m_buffer)
			{
				const VkBufferCopy bc =
				{
					.size = getBufferSize()
				};
				vkCmdCopyBuffer(
					*commandBuffer,
					*stageBuffer,
					*m_buffer,
					1,
					&bc
				);
			}
			safeDestroy(stageBuffer);
		},
		m_size);
}

const IBufferView* BufferStaticVk::getBufferView() const
{
	return &m_bufferView;
}

}
