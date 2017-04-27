/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/SafeDestroy.h"
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
	T_FATAL_ASSERT_M (m_vertexBuffer, L"Render error: Vertex buffer already destroyed.");
	T_FATAL_ASSERT_M (!m_locked, L"Render error: Cannot destroy locked vertex buffer.");
	safeDestroy(m_vertexBuffer);
}

void* VertexBufferCapture::lock()
{
	T_FATAL_ASSERT_M (m_vertexBuffer, L"Render error: Vertex buffer destroyed.");
	T_FATAL_ASSERT_M (!m_locked, L"Render error: Vertex buffer already locked.");
	void* p = m_vertexBuffer->lock();
	if (p)
		m_locked = true;
	return p;
}

void* VertexBufferCapture::lock(uint32_t vertexOffset, uint32_t vertexCount)
{
	T_FATAL_ASSERT_M (m_vertexBuffer, L"Render error: Vertex buffer destroyed.");
	T_FATAL_ASSERT_M (!m_locked, L"Render error: Vertex buffer already locked.");
	T_FATAL_ASSERT_M (vertexOffset + vertexCount <= getBufferSize() / m_vertexSize, L"Render error: Trying to lock vertex buffer out of range.");
	void* p = m_vertexBuffer->lock(vertexOffset, vertexCount);
	if (p)
		m_locked = true;
	return p;
}

void VertexBufferCapture::unlock()
{
	T_FATAL_ASSERT_M (m_vertexBuffer, L"Render error: Vertex buffer destroyed.");
	T_FATAL_ASSERT_M (m_locked, L"Render error: Vertex buffer not locked.");
	m_vertexBuffer->unlock();
	m_locked = false;
	setContentValid(m_vertexBuffer->isContentValid());
}

	}
}
