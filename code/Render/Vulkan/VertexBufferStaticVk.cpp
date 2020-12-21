#include "Core/Misc/SafeDestroy.h"
#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/Buffer.h"
#include "Render/Vulkan/CommandBufferPool.h"
#include "Render/Vulkan/Queue.h"
#include "Render/Vulkan/UtilitiesVk.h"
#include "Render/Vulkan/VertexBufferStaticVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferStaticVk", VertexBufferStaticVk, VertexBufferVk)

VertexBufferStaticVk::VertexBufferStaticVk(
	Context* context,
	Queue* graphicsQueue,
	CommandBufferPool* graphicsCommandPool,
	uint32_t bufferSize,
	const VkVertexInputBindingDescription& vertexBindingDescription,
	const AlignedVector< VkVertexInputAttributeDescription >& vertexAttributeDescriptions,
	uint32_t hash
)
:	VertexBufferVk(bufferSize, vertexBindingDescription, vertexAttributeDescriptions, hash)
,	m_context(context)
,	m_graphicsQueue(graphicsQueue)
,	m_graphicsCommandPool(graphicsCommandPool)
{
}

bool VertexBufferStaticVk::create()
{
	const uint32_t bufferSize = getBufferSize();
	if (!bufferSize)
		return false;

	m_deviceBuffer = new Buffer(m_context);
	if (!m_deviceBuffer->create(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, false, true))
		return false;

	return true;
}

void VertexBufferStaticVk::destroy()
{
	safeDestroy(m_deviceBuffer);
	safeDestroy(m_stageBuffer);
	m_context = nullptr;
}

void* VertexBufferStaticVk::lock()
{
	T_FATAL_ASSERT(m_stageBuffer == nullptr);

	const uint32_t bufferSize = getBufferSize();
	if (!bufferSize)
		return nullptr;

	m_stageBuffer = new Buffer(m_context);
	if (!m_stageBuffer->create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true, true))
		return nullptr;

	return m_stageBuffer->lock();
}

void* VertexBufferStaticVk::lock(uint32_t vertexOffset, uint32_t vertexCount)
{
	T_FATAL_ERROR;
	return nullptr;
}

void VertexBufferStaticVk::unlock()
{
	m_stageBuffer->unlock();

	// Copy staging buffer into vertex buffer.
	VkCommandBuffer commandBuffer = m_graphicsCommandPool->acquireAndBegin();

	VkBufferCopy bc = {};
	bc.size = getBufferSize();
	vkCmdCopyBuffer(
		commandBuffer,
		*m_stageBuffer,
		*m_deviceBuffer,
		1,
		&bc
	);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo si = {};
	si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	si.commandBufferCount = 1;
	si.pCommandBuffers = &commandBuffer;
	m_graphicsQueue->submitAndWait(si);

	m_graphicsCommandPool->release(commandBuffer);

	// Free staging buffer.
	safeDestroy(m_stageBuffer);
}

VkBuffer VertexBufferStaticVk::getVkBuffer() const
{
	return *m_deviceBuffer;
}

	}
}
