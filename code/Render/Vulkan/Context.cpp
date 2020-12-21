#include "Core/Thread/Acquire.h"
#include "Core/Log/Log.h"
#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/Context.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.Context", Context, Object)

Context::Context(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VmaAllocator allocator)
:	m_physicalDevice(physicalDevice)
,	m_logicalDevice(logicalDevice)
,	m_allocator(allocator)
{
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

	// Wait until GPU is idle to ensure resources are not used, or pending, in some queue before destroying them.
	vkDeviceWaitIdle(m_logicalDevice);

	// Take over vector in case more resources are added for cleanup from callbacks.
	AlignedVector< cleanup_fn_t > cleanupFns;
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_cleanupLock);
		cleanupFns.swap(m_cleanupFns);
	}

	// Invoke cleanups.
	log::debug << L"Cleaning " << (int32_t)cleanupFns.size() << L" resources..." << Endl;
	for (const auto& cleanupFn : cleanupFns)
		cleanupFn(this);
}

	}
}
