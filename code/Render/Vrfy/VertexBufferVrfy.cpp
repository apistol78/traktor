#include <cstring>
#include "Core/Memory/Alloc.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/Vrfy/Error.h"
#include "Render/Vrfy/ResourceTracker.h"
#include "Render/Vrfy/VertexBufferVrfy.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

constexpr int32_t c_guardBytes = 16;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferVrfy", VertexBufferVrfy, VertexBuffer)

VertexBufferVrfy::VertexBufferVrfy(ResourceTracker* resourceTracker, VertexBuffer* vertexBuffer, uint32_t bufferSize, uint32_t vertexSize)
:	VertexBuffer(bufferSize)
,	m_resourceTracker(resourceTracker)
,	m_vertexBuffer(vertexBuffer)
,	m_vertexSize(vertexSize)
{
	m_resourceTracker->add(this);
	m_shadow = (uint8_t*)Alloc::acquireAlign(bufferSize + 2 * c_guardBytes, 16, T_FILE_LINE);
	std::memset(m_shadow, 0, bufferSize + 2 * c_guardBytes);
}

VertexBufferVrfy::~VertexBufferVrfy()
{
	verifyGuard();
	Alloc::freeAlign(m_shadow);
	m_resourceTracker->remove(this);
}

void VertexBufferVrfy::destroy()
{
	T_CAPTURE_ASSERT (m_vertexBuffer, L"Vertex buffer already destroyed.");
	T_CAPTURE_ASSERT (!m_locked, L"Cannot destroy locked vertex buffer.");
	verifyGuard();
	safeDestroy(m_vertexBuffer);
}

void* VertexBufferVrfy::lock()
{
	T_CAPTURE_ASSERT (m_vertexBuffer, L"Vertex buffer destroyed.");
	T_CAPTURE_ASSERT (!m_locked, L"Vertex buffer already locked.");

	verifyGuard();

	if (!m_vertexBuffer)
		return nullptr;

	m_device = (uint8_t*)m_vertexBuffer->lock();
	if (m_device)
	{
		m_locked = true;
		std::memset(m_shadow, 0, getBufferSize() + 2 * c_guardBytes);
		return m_shadow + c_guardBytes;
	}
	else
		return nullptr;
}

void VertexBufferVrfy::unlock()
{
	T_CAPTURE_ASSERT (m_vertexBuffer, L"Vertex buffer destroyed.");
	T_CAPTURE_ASSERT (m_locked, L"Vertex buffer not locked.");

	verifyGuard();

	if (!m_vertexBuffer)
		return;

	std::memcpy(m_device, m_shadow + c_guardBytes, getBufferSize());

	m_vertexBuffer->unlock();
	m_locked = false;
}

const IBufferView* VertexBufferVrfy::getBufferView() const
{
	T_CAPTURE_ASSERT (m_vertexBuffer, L"Vertex buffer destroyed.");

	if (!m_vertexBuffer)
		return nullptr;

	return m_vertexBuffer->getBufferView();
}

void VertexBufferVrfy::verifyGuard() const
{
	const uint32_t bufferSize = getBufferSize();
	for (uint32_t i = 0; i < c_guardBytes; ++i)
	{
		T_CAPTURE_ASSERT(m_shadow[i] == 0x00, L"Low guard bytes overwritten.");
		T_CAPTURE_ASSERT(m_shadow[i + c_guardBytes + bufferSize] == 0x00, L"High guard bytes overwritten.");
	}
}

	}
}
