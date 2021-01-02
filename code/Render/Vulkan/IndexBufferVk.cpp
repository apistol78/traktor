#include "Core/Misc/SafeDestroy.h"
#include "Render/Vulkan/IndexBufferVk.h"
#include "Render/Vulkan/Private/ApiLoader.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferVk", IndexBufferVk, IndexBuffer)

IndexBufferVk::IndexBufferVk(
	Context* context,
	IndexType indexType,
	uint32_t bufferSize
)
:	IndexBuffer(indexType, bufferSize)
,	m_context(context)
{
}

bool IndexBufferVk::create()
{
	const uint32_t bufferSize = getBufferSize();
	if (!bufferSize)
		return false;

	m_buffer = new Buffer(m_context);
	if (!m_buffer->create(bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, true, true))
		return false;

	return true;
}

void IndexBufferVk::destroy()
{
	safeDestroy(m_buffer);
}

void* IndexBufferVk::lock()
{
	return m_buffer->lock();
}

void IndexBufferVk::unlock()
{
	m_buffer->unlock();
}

	}
}
