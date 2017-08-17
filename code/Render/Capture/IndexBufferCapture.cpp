/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/SafeDestroy.h"
#include "Render/Capture/IndexBufferCapture.h"
#include "Render/Capture/Error.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferCapture", IndexBufferCapture, IndexBuffer)

IndexBufferCapture::IndexBufferCapture(IndexBuffer* indexBuffer, IndexType indexType, uint32_t bufferSize)
:	IndexBuffer(indexType, bufferSize)
,	m_indexBuffer(indexBuffer)
,	m_locked(false)
{
}

void IndexBufferCapture::destroy()
{
	T_CAPTURE_ASSERT (m_indexBuffer, L"Index buffer already destroyed.");
	T_CAPTURE_ASSERT (!m_locked, L"Cannot destroy locked index buffer.");
	safeDestroy(m_indexBuffer);
}

void* IndexBufferCapture::lock()
{
	T_CAPTURE_ASSERT (m_indexBuffer, L"Index buffer destroyed.");
	T_CAPTURE_ASSERT (!m_locked, L"Index buffer already locked.");
	
	if (!m_indexBuffer)
		return 0;

	void* p = m_indexBuffer->lock();
	if (p)
		m_locked = true;

	return p;
}
	
void IndexBufferCapture::unlock()
{
	T_CAPTURE_ASSERT (m_indexBuffer, L"Index buffer destroyed.");
	T_CAPTURE_ASSERT (m_locked, L"Index buffer not locked.");

	if (!m_indexBuffer)
		return;

	m_indexBuffer->unlock();
	m_locked = false;
}

	}
}
