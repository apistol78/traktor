#include <algorithm>
#include "Core/Log/Log.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/Buffer.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/UniformBufferPool.h"
#include "Render/Vulkan/Private/Utilities.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

int32_t freeIndex(uint32_t size)
{
	int32_t index = (int32_t)(size / 16);
	if (index > 63)
		index = 63;
	return index;
}

		}
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.UniformBufferPool", UniformBufferPool, Object)

UniformBufferPool::UniformBufferPool(Context* context)
:	m_context(context)
,	m_counter(0)
{
}

UniformBufferPool::~UniformBufferPool()
{
	for (auto& chain : m_free)
	{
		for (auto& c : chain)
		{
			c.buffer->unlock();
			c.buffer->destroy();
		}
	}
	for (auto& chain : m_released)
	{
		for (auto& c : chain)
		{
			c.buffer->unlock();
			c.buffer->destroy();
		}
	}
}

bool UniformBufferPool::acquire(
	uint32_t size,
	Ref< Buffer >& inoutBuffer,
	void*& inoutMappedPtr
)
{
	if (inoutBuffer != 0)
	{
		BufferChain& bc = m_released[m_counter].push_back();
		bc.size = size;
		bc.buffer = inoutBuffer;
		bc.mappedPtr = inoutMappedPtr;

		inoutBuffer = nullptr;
		inoutMappedPtr = nullptr;
	}

	// Get free list from size.
	auto& free = m_free[freeIndex(size)];

	// Find matching buffer from free list.
	auto it = std::find_if(free.begin(), free.end(), [=](const BufferChain& bc) { return bc.size == size; });
	if (it != free.end())
	{
		inoutBuffer = it->buffer;
		inoutMappedPtr = it->mappedPtr;
		
		if (it != free.end() - 1)
			*it = free.back();

		free.pop_back();
	}
	else
	{
		inoutBuffer = new Buffer(m_context);
		inoutBuffer->create(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, true, true);
		inoutMappedPtr = inoutBuffer->lock();
	}

	return true;
}

void UniformBufferPool::collect()
{
	uint32_t c = (m_counter + 1) % MaxPendingFrames;
	if (!m_released[c].empty())
	{
		for (const auto& released : m_released[c])
		{
			auto& free = m_free[freeIndex(released.size)];
			free.push_back(released);
		}
		m_released[c].resize(0);
	}
	m_counter = c;
}

	}
}