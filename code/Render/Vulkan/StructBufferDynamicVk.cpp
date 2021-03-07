#include "Core/Misc/SafeDestroy.h"
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
	uint32_t storageBufferOffsetAlignment = (uint32_t)deviceProperties.limits.minStorageBufferOffsetAlignment;

	m_alignedBufferSize = alignUp(bufferSize, storageBufferOffsetAlignment);
	m_inFlightCount = inFlightCount;

	m_buffer = new Buffer(m_context);
	if (!m_buffer->create(m_alignedBufferSize * inFlightCount, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, true, true))
		return false;

	return true;
}

void StructBufferDynamicVk::destroy()
{
	safeDestroy(m_buffer);
	m_context = nullptr;
}

void* StructBufferDynamicVk::lock()
{
	uint8_t* ptr = (uint8_t*)m_buffer->lock();
	if (!ptr)
		return nullptr;

	return ptr + m_index * m_alignedBufferSize;
}

void StructBufferDynamicVk::unlock()
{
	m_buffer->unlock();
	m_offset = m_index * m_alignedBufferSize;
	m_index = (m_index + 1) % m_inFlightCount;
}

	}
}