#include "Render/Vulkan/StructBufferDynamicVk.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/Context.h"

namespace traktor
{
	namespace render
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.StructBufferDynamicVk", StructBufferDynamicVk, StructBufferVk)

StructBufferDynamicVk::StructBufferDynamicVk(Context* context, uint32_t bufferSize, uint32_t& instances)
:	StructBufferVk(context, bufferSize, instances)
{
}

StructBufferDynamicVk::~StructBufferDynamicVk()
{
	destroy();
}

bool StructBufferDynamicVk::create(int32_t inFlightCount)
{
	const uint32_t bufferSize = getBufferSize();
	if (!bufferSize)
		return false;

	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(m_context->getPhysicalDevice(), &deviceProperties);
	const uint32_t storageBufferOffsetAlignment = (uint32_t)deviceProperties.limits.minStorageBufferOffsetAlignment;

	// Add an extra buffer since threaded rendering might queue an extra frame worth of rendering.
	m_inFlightCount = inFlightCount + 1;
	m_range = alignUp(bufferSize, storageBufferOffsetAlignment);
	
	const uint32_t size = m_range * m_inFlightCount;

	m_buffer = new Buffer(m_context);
	if (!m_buffer->create(size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, true, true))
		return false;

	m_bufferViews = new BufferViewVk [m_inFlightCount];
	for (uint32_t i = 0; i < m_inFlightCount; ++i)
	{
		m_bufferViews[i] = BufferViewVk(
			*m_buffer,
			i * m_range,
			m_range,
			size
		);
	}

	m_ptr = (uint8_t*)m_buffer->lock();
	if (!m_ptr)
		return false;

	return true;
}

void StructBufferDynamicVk::destroy()
{
	if (m_bufferViews)
	{
		delete[] m_bufferViews;
		m_bufferViews = nullptr;
	}

	if (m_buffer)
	{
		m_buffer->unlock();
		m_buffer->destroy();
		m_buffer = nullptr;
	}

	m_context = nullptr;
	m_ptr = nullptr;
}

void* StructBufferDynamicVk::lock()
{
	return m_ptr + m_index * m_range;
}

void StructBufferDynamicVk::unlock()
{
	m_view = m_index;
	m_index = (m_index + 1) % m_inFlightCount;
}

const IBufferView* StructBufferDynamicVk::getBufferView() const
{
	return &m_bufferViews[m_view];
}

	}
}