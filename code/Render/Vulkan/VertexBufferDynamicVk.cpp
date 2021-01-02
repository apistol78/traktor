#include "Render/Vulkan/VertexBufferDynamicVk.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/Buffer.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferDynamicVk", VertexBufferDynamicVk, VertexBufferVk)

VertexBufferDynamicVk::VertexBufferDynamicVk(
	Context* context,
	uint32_t bufferSize,
	const VkVertexInputBindingDescription& vertexBindingDescription,
	const AlignedVector< VkVertexInputAttributeDescription >& vertexAttributeDescriptions,
	uint32_t hash
)
:	VertexBufferVk(bufferSize, vertexBindingDescription, vertexAttributeDescriptions, hash)
,	m_context(context)
,	m_index(0)
{
}

VertexBufferDynamicVk::~VertexBufferDynamicVk()
{
	destroy();
}

bool VertexBufferDynamicVk::create(int32_t inFlightCount)
{
	const uint32_t bufferSize = getBufferSize();
	if (!bufferSize)
		return false;

	m_buffers.resize(inFlightCount);
	for (int32_t i = 0; i < inFlightCount; ++i)
	{
		m_buffers[i] = new Buffer(m_context);
		m_buffers[i]->create(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, true, true);
	}

	return true;
}

void VertexBufferDynamicVk::destroy()
{
	for (auto buffer : m_buffers)
		buffer->destroy();
	m_buffers.clear();
	m_context = nullptr;
}

void* VertexBufferDynamicVk::lock()
{
	int32_t next = (m_index + 1) % (int32_t)m_buffers.size();
	return m_buffers[next]->lock();
}

void* VertexBufferDynamicVk::lock(uint32_t vertexOffset, uint32_t vertexCount)
{
	T_FATAL_ERROR;
	return nullptr;
}

void VertexBufferDynamicVk::unlock()
{
	int32_t next = (m_index + 1) % (int32_t)m_buffers.size();
	m_buffers[next]->unlock();
	m_index = next;
}

VkBuffer VertexBufferDynamicVk::getVkBuffer() const
{
	return *m_buffers[m_index];
}

	}
}
