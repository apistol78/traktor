#include "Core/Thread/Acquire.h"
#include "Core/Log/Log.h"
#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/CommandBuffer.h"
#include "Render/Vulkan/Context.h"
#include "Render/Vulkan/Queue.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.Context", Context, Object)

Context::Context(
	VkPhysicalDevice physicalDevice,
	VkDevice logicalDevice,
	VmaAllocator allocator,
	uint32_t graphicsQueueIndex,
	uint32_t computeQueueIndex
)
:	m_physicalDevice(physicalDevice)
,	m_logicalDevice(logicalDevice)
,	m_allocator(allocator)
{
	m_graphicsQueue = Queue::create(this, graphicsQueueIndex);
	m_computeQueue = Queue::create(this, computeQueueIndex);
}

void Context::addDeferredCleanup(const cleanup_fn_t& fn)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_cleanupLock);
	m_cleanupFns.push_back(fn);
}

void Context::performCleanup()
{
	if (m_cleanupFns.empty())
		return;

	// Take over vector in case more resources are added for cleanup from callbacks.
	// Wait until GPU is idle to ensure resources are not used, or pending, in some queue before destroying them.
	AlignedVector< cleanup_fn_t > cleanupFns;
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_cleanupLock);
		vkDeviceWaitIdle(m_logicalDevice);
		cleanupFns.swap(m_cleanupFns);
	}

	// Invoke cleanups.
	for (const auto& cleanupFn : cleanupFns)
		cleanupFn(this);
}

	}
}
