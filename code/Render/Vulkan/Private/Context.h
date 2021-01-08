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

	virtual ~Context();

	void addDeferredCleanup(const cleanup_fn_t& fn);

	bool needCleanup() const;

	void performCleanup();

	VkPhysicalDevice getPhysicalDevice() const { return m_physicalDevice; }

	VkDevice getLogicalDevice() const { return m_logicalDevice; }

	VmaAllocator getAllocator() const { return m_allocator; }

	VkPipelineCache getPipelineCache() const { return m_pipelineCache; }

	VkDescriptorPool getDescriptorPool() const { return m_descriptorPool; }

	uint32_t getDescriptorPoolRevision() const { return m_descriptorPoolRevision; }

	Queue* getGraphicsQueue() const { return m_graphicsQueue; }

	Queue* getComputeQueue() const { return m_computeQueue; }

private:
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_logicalDevice;
	VmaAllocator m_allocator;
	VkPipelineCache m_pipelineCache;
	VkDescriptorPool m_descriptorPool;
	uint32_t m_descriptorPoolRevision;
	Ref< Queue > m_graphicsQueue;
	Ref< Queue > m_computeQueue;
	Semaphore m_cleanupLock;
	AlignedVector< cleanup_fn_t > m_cleanupFns;
};

	}
}
