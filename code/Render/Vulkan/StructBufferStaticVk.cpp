#include "Core/Misc/SafeDestroy.h"
#include "Render/Vulkan/StructBufferStaticVk.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/Queue.h"

namespace traktor
{
	namespace render
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.StructBufferStaticVk", StructBufferStaticVk, StructBufferVk)

StructBufferStaticVk::StructBufferStaticVk(Context* context, uint32_t bufferSize, uint32_t& instances)
:	StructBufferVk(context, bufferSize, instances)
{
}

StructBufferStaticVk::~StructBufferStaticVk()
{
	destroy();
}

bool StructBufferStaticVk::create()
{
	const uint32_t bufferSize = getBufferSize();
	if (!bufferSize)
		return false;

	m_buffer = new Buffer(m_context);
	if (!m_buffer->create(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, false, true))
		return false;

	m_bufferView = BufferViewVk(*m_buffer, 0, bufferSize, bufferSize);
	m_size = bufferSize;
	return true;
}

void StructBufferStaticVk::destroy()
{
	safeDestroy(m_buffer);
	safeDestroy(m_stageBuffer);
	m_context = nullptr;
}

void* StructBufferStaticVk::lock()
{
	T_FATAL_ASSERT(m_stageBuffer == nullptr);

	m_stageBuffer = new Buffer(m_context);
	if (!m_stageBuffer->create(m_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true, true))
		return nullptr;

	return m_stageBuffer->lock();
}

void StructBufferStaticVk::unlock()
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

const IBufferView* StructBufferStaticVk::getBufferView() const
{
	return &m_bufferView;
}

	}
}