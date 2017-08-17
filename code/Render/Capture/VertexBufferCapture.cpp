/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/SafeDestroy.h"
#include "Render/Capture/Error.h"
#include "Render/Capture/VertexBufferCapture.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferCapture", VertexBufferCapture, VertexBuffer)

VertexBufferCapture::VertexBufferCapture(VertexBuffer* vertexBuffer, uint32_t bufferSize, uint32_t vertexSize)
:	VertexBuffer(bufferSize)
,	m_vertexBuffer(vertexBuffer)
,	m_vertexSize(vertexSize)
,	m_locked(false)
{
	setContentValid(true);
}

void VertexBufferCapture::destroy()
{
	T_CAPTURE_ASSERT (m_vertexBuffer, L"Vertex buffer already destroyed.");
	T_CAPTURE_ASSERT (!m_locked, L"Cannot destroy locked vertex buffer.");
	safeDestroy(m_vertexBuffer);
}

void* VertexBufferCapture::lock()
{
	T_CAPTURE_ASSERT (m_vertexBuffer, L"Vertex buffer destroyed.");
	T_CAPTURE_ASSERT (!m_locked, L"Vertex buffer already locked.");

	if (!m_vertexBuffer)
		return 0;

	void* p = m_vertexBuffer->lock();
	if (p)
		m_locked = true;

	return p;
}

void* VertexBufferCapture::lock(uint32_t vertexOffset, uint32_t vertexCount)
{
	T_CAPTURE_ASSERT (m_vertexBuffer, L"Vertex buffer destroyed.");
	T_CAPTURE_ASSERT (!m_locked, L"Vertex buffer already locked.");
	T_CAPTURE_ASSERT (vertexOffset + vertexCount <= getBufferSize() / m_vertexSize, L"Trying to lock vertex buffer out of range.");

	if (!m_vertexBuffer)
		return 0;

	void* p = m_vertexBuffer->lock(vertexOffset, vertexCount);
	if (p)
		m_locked = true;

	return p;
}

void VertexBufferCapture::unlock()
{
	T_CAPTURE_ASSERT (m_vertexBuffer, L"Vertex buffer destroyed.");
	T_CAPTURE_ASSERT (m_locked, L"Vertex buffer not locked.");

	if (!m_vertexBuffer)
		return;

	m_vertexBuffer->unlock();
	m_locked = false;

	setContentValid(m_vertexBuffer->isContentValid());
}

	}
}
