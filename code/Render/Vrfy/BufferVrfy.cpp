/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Core/Debug/CallStack.h"
#include "Core/Memory/Alloc.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/Vrfy/BufferVrfy.h"
#include "Render/Vrfy/Error.h"
#include "Render/Vrfy/ResourceTracker.h"

#define T_VRFY_CHECK_UNTOUCHED 0

namespace traktor::render
{
	namespace
	{

constexpr int32_t c_guardBytes = 16;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.BufferVrfy", BufferVrfy, Buffer)

BufferVrfy::BufferVrfy(ResourceTracker* resourceTracker, Buffer* buffer, uint32_t bufferSize)
:	Buffer(bufferSize)
,	m_resourceTracker(resourceTracker)
,	m_buffer(buffer)
{
	m_resourceTracker->add(this);

	m_shadow = (uint8_t*)Alloc::acquireAlign(bufferSize + 2 * c_guardBytes, 16, T_FILE_LINE);
	std::memset(m_shadow, 0, bufferSize + 2 * c_guardBytes);

	getCallStack(8, m_callstack, 2);

	for (int32_t i = 0; i < sizeof_array(m_bufferViews); ++i)
		m_bufferViews[i] = BufferViewVrfy(this);
}

BufferVrfy::~BufferVrfy()
{
	verifyGuard();
	verifyUntouched();
	Alloc::freeAlign(m_shadow);
	m_resourceTracker->remove(this);
}

void BufferVrfy::destroy()
{
	T_CAPTURE_ASSERT (m_buffer, L"Buffer already destroyed.");
	T_CAPTURE_ASSERT (!m_locked, L"Cannot destroy locked struct buffer.");
	verifyGuard();
	verifyUntouched();
	safeDestroy(m_buffer);
}

void* BufferVrfy::lock()
{
	T_CAPTURE_ASSERT (m_buffer, L"Buffer destroyed.");
	T_CAPTURE_ASSERT (!m_locked, L"Buffer already locked.");

	verifyGuard();
	verifyUntouched();	

	if (!m_buffer)
		return nullptr;

	m_device = (uint8_t*)m_buffer->lock();
	if (m_device)
	{
		m_locked = true;
		std::memset(m_shadow, 0, getBufferSize() + 2 * c_guardBytes);
		return m_shadow + c_guardBytes;
	}
	else
		return nullptr;
}

void BufferVrfy::unlock()
{
	T_CAPTURE_ASSERT(m_buffer, L"Buffer destroyed.");
	T_CAPTURE_ASSERT(m_locked, L"Buffer not locked.");

	verifyGuard();

	if (!m_buffer)
		return;

	std::memcpy(m_device, m_shadow + c_guardBytes, getBufferSize());
	std::memset(m_shadow, 0, getBufferSize() + 2 * c_guardBytes);

	m_buffer->unlock();
	m_locked = false;
}

const IBufferView* BufferVrfy::getBufferView() const
{
	T_CAPTURE_ASSERT(m_buffer, L"Buffer destroyed.");
	auto& bv = m_bufferViews[m_bufferViewIndex];
	m_bufferViewIndex = (m_bufferViewIndex + 1) % sizeof_array(m_bufferViews);
	bv.m_wrappedBufferView = m_buffer->getBufferView();
	return &bv;
}

void BufferVrfy::verifyGuard() const
{
	const uint32_t bufferSize = getBufferSize();
	for (uint32_t i = 0; i < c_guardBytes; ++i)
	{
		T_CAPTURE_ASSERT(m_shadow[i] == 0x00, L"Low guard bytes overwritten.");
		T_CAPTURE_ASSERT(m_shadow[i + c_guardBytes + bufferSize] == 0x00, L"High guard bytes overwritten.");
	}
}

void BufferVrfy::verifyUntouched() const
{
#if defined(T_VRFY_CHECK_UNTOUCHED)
	const uint32_t bufferSize = getBufferSize();
	for (uint32_t i = 0; i < bufferSize; ++i)
		T_CAPTURE_ASSERT(m_shadow[i + c_guardBytes] == 0x00, L"Memory touched outside of lock/unlock region.");
#endif
}

}
