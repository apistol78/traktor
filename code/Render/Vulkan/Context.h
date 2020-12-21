#pragma once

#include <functional>
#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Thread/Semaphore.h"
#include "Render/Vulkan/ApiHeader.h"

namespace traktor
{
	namespace render
	{

class Context : public Object
{
	T_RTTI_CLASS;

public:
	typedef std::function< void(Context*) > cleanup_fn_t;

	explicit Context(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VmaAllocator allocator);

	void addDeferredCleanup(const cleanup_fn_t& fn); 

	void performCleanup();

	VkPhysicalDevice getPhysicalDevice() const { return m_physicalDevice; }

	VkDevice getLogicalDevice() const { return m_logicalDevice; }

	VmaAllocator getAllocator() const { return m_allocator; }

private:
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_logicalDevice;
	VmaAllocator m_allocator;
	Semaphore m_cleanupLock;
	AlignedVector< cleanup_fn_t > m_cleanupFns;
};

	}
}
