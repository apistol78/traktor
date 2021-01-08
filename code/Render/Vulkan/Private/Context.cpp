
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/System/OS.h"
#include "Core/Thread/Acquire.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/Queue.h"

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
,	m_pipelineCache(0)
,	m_descriptorPool(0)
,	m_descriptorPoolRevision(0)
{
	AlignedVector< uint8_t > buffer;

	// Create queues.
	m_graphicsQueue = Queue::create(this, graphicsQueueIndex);
	m_computeQueue = (computeQueueIndex != graphicsQueueIndex) ? Queue::create(this, computeQueueIndex) : m_graphicsQueue;

	// Create pipeline cache.
	VkPipelineCacheCreateInfo pcci = {};
	pcci.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	pcci.flags = 0;
	pcci.initialDataSize = 0;
	pcci.pInitialData = nullptr;

	StringOutputStream ss;
	ss << OS::getInstance().getWritableFolderPath() << L"/Traktor/Vulkan/Pipeline.cache";

	Ref< IStream > file = FileSystem::getInstance().open(ss.str(), File::FmRead);
	if (file)
	{
	 	uint32_t size = (uint32_t)file->available();
		buffer.resize(size);
	 	file->read(buffer.ptr(), size);
	 	file->close();

		pcci.initialDataSize = size;
		pcci.pInitialData = buffer.c_ptr();
	}

	vkCreatePipelineCache(
		m_logicalDevice,
		&pcci,
		nullptr,
		&m_pipelineCache
	);

	// Create descriptor set pool.
	VkDescriptorPoolSize dps[4];
	dps[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	dps[0].descriptorCount = 40000;
	dps[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;
	dps[1].descriptorCount = 40000;
	dps[2].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	dps[2].descriptorCount = 40000;
	dps[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	dps[3].descriptorCount = 4000;

	VkDescriptorPoolCreateInfo dpci = {};
	dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	dpci.pNext = nullptr;
	dpci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	dpci.maxSets = 16000;
	dpci.poolSizeCount = sizeof_array(dps);
	dpci.pPoolSizes = dps;
	vkCreateDescriptorPool(m_logicalDevice, &dpci, nullptr, &m_descriptorPool);
}

Context::~Context()
{
	// Save pipeline cache.
	size_t size = 0;
	vkGetPipelineCacheData(m_logicalDevice, m_pipelineCache, &size, nullptr);
	if (size > 0)
	{
		AlignedVector< uint8_t > buffer(size);
		vkGetPipelineCacheData(m_logicalDevice, m_pipelineCache, &size, buffer.ptr());

		StringOutputStream ss;
		ss << OS::getInstance().getWritableFolderPath() << L"/Traktor/Vulkan/Pipeline.cache";

		FileSystem::getInstance().makeAllDirectories(Path(ss.str()).getPathOnly());

		Ref< IStream > file = FileSystem::getInstance().open(ss.str(), File::FmWrite);
		if (file)
		{
	 		file->write(buffer.c_ptr(), size);
	 		file->close();
		}
	}

	// Destroy descriptor pool.
	if (m_descriptorPool != 0)
	{
		vkDestroyDescriptorPool(m_logicalDevice, m_descriptorPool, nullptr);
		m_descriptorPool = 0;
	}
}

void Context::addDeferredCleanup(const cleanup_fn_t& fn)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_cleanupLock);
	m_cleanupFns.push_back(fn);
}

bool Context::needCleanup() const
{
	return !m_cleanupFns.empty();
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

	// Reset descriptor pool since we need to ensure programs clear their cached descriptor sets.
	vkResetDescriptorPool(m_logicalDevice, m_descriptorPool, 0);
	m_descriptorPoolRevision++;
}

	}
}
