#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/StructBufferVk.h"

namespace traktor
{
	namespace render
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.StructBufferVk", StructBufferVk, StructBuffer)

StructBufferVk::StructBufferVk(
	uint32_t bufferSize,
	Buffer&& buffer
)
:	StructBuffer(bufferSize)
,	m_buffer(buffer)
{
}

void StructBufferVk::destroy()
{
	m_buffer.destroy();
}

void* StructBufferVk::lock()
{
	return m_buffer.lock();
}

void* StructBufferVk::lock(uint32_t structOffset, uint32_t structCount)
{
	return nullptr;
}

void StructBufferVk::unlock()
{
	return m_buffer.unlock();
}

	}
}