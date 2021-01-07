#include "Core/Misc/SafeDestroy.h"
#include "Render/Vulkan/VertexBufferStaticVk.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/Buffer.h"
#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/Queue.h"
#include "Render/Vulkan/Private/Utilities.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferStaticVk", VertexBufferStaticVk, VertexBufferVk)

VertexBufferStaticVk::VertexBufferStaticVk(
	Context* context,
	uint32_t bufferSize,
	const VkVertexInputBindingDescription& vertexBindingDescription,
	const AlignedVector< VkVertexInputAttributeDescription >& vertexAttributeDescriptions,
	uint32_t hash,
	uint32_t& instances
)
:	VertexBufferVk(bufferSize, vertexBindingDescription, vertexAttributeDescriptions, hash, instances)
,	m_context(context)
{
}

VertexBufferStaticVk::~VertexBufferStaticVk()
{
	destroy();
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

	auto commandBuffer = m_context->getGraphicsQueue()->acquireCommandBuffer();

	VkBufferCopy bc = {};
	bc.size = getBufferSize();
	vkCmdCopyBuffer(
		*commandBuffer,
		*m_stageBuffer,
		*m_deviceBuffer,
		1,
		&bc
	);

	commandBuffer->submitAndWait();

	// Free staging buffer.
	safeDestroy(m_stageBuffer);
}

VkBuffer VertexBufferStaticVk::getVkBuffer() const
{
	return *m_deviceBuffer;
}

	}
}
