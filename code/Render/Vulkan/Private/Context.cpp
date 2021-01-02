
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
{
	AlignedVector< uint8_t > buffer;

	m_graphicsQueue = Queue::create(this, graphicsQueueIndex);
	m_computeQueue = Queue::create(this, computeQueueIndex);

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
}

Context::~Context()
{
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
