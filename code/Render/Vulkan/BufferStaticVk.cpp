#include "Core/Misc/SafeDestroy.h"
#include "Render/Vulkan/BufferStaticVk.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/Queue.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.BufferStaticVk", BufferStaticVk, BufferVk)

BufferStaticVk::BufferStaticVk(Context* context, uint32_t elementCount, uint32_t elementSize, uint32_t& instances)
:	BufferVk(context, elementCount, elementSize, instances)
{
}

BufferStaticVk::~BufferStaticVk()
{
	destroy();
}

bool BufferStaticVk::create(uint32_t usageBits)
{
	const uint32_t bufferSize = getBufferSize();
	if (!bufferSize)
		return false;

	m_buffer = new ApiBuffer(m_context);
	if (!m_buffer->create(bufferSize, usageBits | VK_BUFFER_USAGE_TRANSFER_DST_BIT, false, true))
		return false;

	m_bufferView = BufferViewVk(*m_buffer, 0, bufferSize, bufferSize);
	m_size = bufferSize;
	return true;
}

void BufferStaticVk::destroy()
{
	safeDestroy(m_buffer);
	safeDestroy(m_stageBuffer);
	m_context = nullptr;
}

void* BufferStaticVk::lock()
{
	T_FATAL_ASSERT(m_stageBuffer == nullptr);

	m_stageBuffer = new ApiBuffer(m_context);
	if (!m_stageBuffer->create(m_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true, true))
		return nullptr;

	return m_stageBuffer->lock();
}

void BufferStaticVk::unlock()
{
	m_stageBuffer->unlock();

	auto commandBuffer = m_context->getGraphicsQueue()->acquireCommandBuffer(T_FILE_LINE_W);

	VkBufferCopy bc = {};
	bc.size = getBufferSize();
	vkCmdCopyBuffer(
		*commandBuffer,
		*m_stageBuffer,
		*m_buffer,
		1,
		&bc
	);

	commandBuffer->submitAndWait();

	// Free staging buffer.
	safeDestroy(m_stageBuffer);
}

const IBufferView* BufferStaticVk::getBufferView() const
{
	return &m_bufferView;
}

}
