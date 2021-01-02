#pragma once

#include <functional>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Thread/Semaphore.h"
#include "Render/Vulkan/Private/ApiHeader.h"

namespace traktor
{
	namespace render
	{

class Queue;

class Context : public Object
{
	T_RTTI_CLASS;

public:
	typedef std::function< void(Context*) > cleanup_fn_t;

	explicit Context(
		VkPhysicalDevice physicalDevice,
		VkDevice logicalDevice,
		VmaAllocator allocator,
		uint32_t graphicsQueueIndex,
		uint32_t computeQueueIndex
	);

	void addDeferredCleanup(const cleanup_fn_t& fn);

	void performCleanup();

	VkPhysicalDevice getPhysicalDevice() const { return m_physicalDevice; }

	VkDevice getLogicalDevice() const { return m_logicalDevice; }

	VmaAllocator getAllocator() const { return m_allocator; }

	Queue* getGraphicsQueue() const { return m_graphicsQueue; }

	Queue* getComputeQueue() const { return m_computeQueue; }

private:
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_logicalDevice;
	VmaAllocator m_allocator;

	Ref< Queue > m_graphicsQueue;
	Ref< Queue > m_computeQueue;

	Semaphore m_cleanupLock;
	AlignedVector< cleanup_fn_t > m_cleanupFns;
};

	}
}
