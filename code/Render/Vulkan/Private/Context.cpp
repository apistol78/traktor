/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Vulkan/Private/Context.h"

#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Core/System/OS.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Atomic.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Timer/Profiler.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Queue.h"
#include "Render/Vulkan/Private/RenderPassCache.h"
#include "Render/Vulkan/Private/UniformBufferPool.h"
#include "Render/Vulkan/Private/Utilities.h"
#include "Render/Vulkan/ProgramVk.h"
#include "Render/Vulkan/RenderTargetSetVk.h"
#include "Render/Vulkan/VertexLayoutVk.h"

#include <algorithm>
#include <cstring>
#include <sstream>
#include <string>
#include <thread>

namespace traktor::render
{

namespace
{

/*! Amount of staging memory queued uploads may hold back before being flushed.
 * Without a cap, resources created quicker than uploads are flushed would keep all their staging buffers alive.
 */
constexpr uint32_t c_maxPendingUploadSize = 32 * 1024 * 1024;

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.Context", Context, Object)

Context::Context(
	VkInstance instance,
	VkPhysicalDevice physicalDevice,
	VkDevice logicalDevice,
	VmaAllocator allocator,
	uint32_t graphicsQueueIndex,
	uint32_t computeQueueIndex,
	bool rayTracing,
	bool smoothLines,
	bool hostQueryReset)
	: m_instance(instance)
	, m_physicalDevice(physicalDevice)
	, m_logicalDevice(logicalDevice)
	, m_allocator(allocator)
	, m_graphicsQueueIndex(graphicsQueueIndex)
	, m_computeQueueIndex(computeQueueIndex)
	, m_rayTracing(rayTracing)
	, m_smoothLines(smoothLines)
	, m_hostQueryReset(hostQueryReset)
	, m_sampledResourceIndexAllocator(0, MaxBindlessResources - 1)
	, m_storageResourceIndexAllocator(0, MaxBindlessResources - 1)
	, m_bufferResourceIndexAllocator(0, MaxBindlessResources - 1)
{
}

Context::~Context()
{
	// Nothing submitted may still be executing when the objects it references are destroyed.
	vkDeviceWaitIdle(m_logicalDevice);

	// Release upload command buffers before the pool they were allocated from.
	if (m_graphicsQueue)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_graphicsQueue->m_lock);
		retireUploads(true);
	}
	if (m_uploadCommandPool != 0)
	{
		vkDestroyCommandPool(m_logicalDevice, m_uploadCommandPool, nullptr);
		m_uploadCommandPool = 0;
	}

	// Destroy pipelines.
	for (auto& pipeline : m_pipelines)
		vkDestroyPipeline(m_logicalDevice, pipeline.second, nullptr);
	m_pipelines.clear();
	for (auto& pipeline : m_computePipelines)
		vkDestroyPipeline(m_logicalDevice, pipeline.second, nullptr);
	m_computePipelines.clear();

	// Destroy render passes.
	m_renderPassCache = nullptr;

	// Destroy uniform buffer pools.
	for (int32_t i = 0; i < sizeof_array(m_uniformBufferPools); ++i)
	{
		if (m_uniformBufferPools[i])
		{
			m_uniformBufferPools[i]->destroy();
			m_uniformBufferPools[i] = nullptr;
		}
	}

	// Destroy descriptor pools; bindless sets are released together with their pool.
	if (m_descriptorPool != 0)
	{
		vkDestroyDescriptorPool(m_logicalDevice, m_descriptorPool, nullptr);
		m_descriptorPool = 0;
	}
	if (m_bindlessDescriptorPool != 0)
	{
		vkDestroyDescriptorPool(m_logicalDevice, m_bindlessDescriptorPool, nullptr);
		m_bindlessDescriptorPool = 0;
	}

	const VkDescriptorSetLayout bindlessLayouts[] = { m_bindlessTexturesDescriptorLayout, m_bindlessImagesDescriptorLayout, m_bindlessBuffersDescriptorLayout };
	for (auto layout : bindlessLayouts)
		if (layout != 0)
			vkDestroyDescriptorSetLayout(m_logicalDevice, layout, nullptr);

	// Destroy upload semaphore.
	if (m_uploadSemaphore != VK_NULL_HANDLE)
	{
		vkDestroySemaphore(m_logicalDevice, m_uploadSemaphore, nullptr);
		m_uploadSemaphore = VK_NULL_HANDLE;
	}
}

bool Context::create()
{
	AlignedVector< uint8_t > buffer;

	// Create queues.
	m_graphicsQueue = Queue::create(this, m_graphicsQueueIndex);
	if (m_computeQueueIndex == m_graphicsQueueIndex)
		m_computeQueue = m_graphicsQueue;
	else
		m_computeQueue = Queue::create(this, m_computeQueueIndex);

	// Create upload timeline semaphore, signalled by each upload submission.
	const VkSemaphoreTypeCreateInfo stci = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
		.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
		.initialValue = 0
	};
	const VkSemaphoreCreateInfo sci = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = &stci
	};
	if (vkCreateSemaphore(m_logicalDevice, &sci, nullptr, &m_uploadSemaphore) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; failed to create upload semaphore." << Endl;
		return false;
	}

	// Upload command buffers are used from any thread, thus a pool of their own guarded by the graphics queue lock.
	if ((m_uploadCommandPool = m_graphicsQueue->createCommandPool()) == 0)
	{
		log::error << L"Failed to create Vulkan; failed to create upload command pool." << Endl;
		return false;
	}

	// Create pipeline cache.
	VkPipelineCacheCreateInfo pcci = {};
	pcci.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	pcci.flags = 0;
	pcci.initialDataSize = 0;
	pcci.pInitialData = nullptr;

	StringOutputStream ss;
#if defined(__IOS__)
	ss << OS::getInstance().getUserHomePath() << L"/Library/Caches/Traktor/Vulkan/Pipeline.cache";
#else
	ss << OS::getInstance().getWritableFolderPath() << L"/Traktor/Vulkan/Pipeline.cache";
#endif

	Ref< IStream > file = FileSystem::getInstance().open(ss.str(), File::FmRead);
	if (file)
	{
		const uint32_t size = (uint32_t)file->available();
		buffer.resize(size);
		file->read(buffer.ptr(), size);
		file->close();

		pcci.initialDataSize = size;
		pcci.pInitialData = buffer.c_ptr();

		log::debug << L"Pipeline cache \"" << ss.str() << L"\" loaded successfully." << Endl;
	}
	else
		log::debug << L"No pipeline cache found; creating new cache." << Endl;

	vkCreatePipelineCache(
		m_logicalDevice,
		&pcci,
		nullptr,
		&m_pipelineCache);

	// Create descriptor set pool for per-program sets; bindless sets have a pool of their own.
	StaticVector< VkDescriptorPoolSize, 6 > dps;
	dps.push_back({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 80000 });
	dps.push_back({ VK_DESCRIPTOR_TYPE_SAMPLER, 80000 });
	dps.push_back({ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, MaxBindlessResources });
	dps.push_back({ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 8000 });
	dps.push_back({ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MaxBindlessResources });
	if (m_rayTracing)
		dps.push_back({ VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 8000 });

	const VkDescriptorPoolCreateInfo dpci = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
		.maxSets = 32000,
		.poolSizeCount = (uint32_t)dps.size(),
		.pPoolSizes = dps.c_ptr()
	};
	if (vkCreateDescriptorPool(m_logicalDevice, &dpci, nullptr, &m_descriptorPool) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; failed to create descriptor pool." << Endl;
		return false;
	}

	const VkDescriptorPoolSize bindlessDps[] = {
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, MaxBindlessResources },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MaxBindlessResources },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MaxBindlessResources }
	};

	const VkDescriptorPoolCreateInfo bindlessDpci = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT,
		.maxSets = sizeof_array(bindlessDps),
		.poolSizeCount = sizeof_array(bindlessDps),
		.pPoolSizes = bindlessDps
	};
	if (vkCreateDescriptorPool(m_logicalDevice, &bindlessDpci, nullptr, &m_bindlessDescriptorPool) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; failed to create bindless descriptor pool." << Endl;
		return false;
	}

	// Create uniform buffer pools.
	m_uniformBufferPools[0] = new UniformBufferPool(this, 1000, L"Once");
	m_uniformBufferPools[1] = new UniformBufferPool(this, 10000, L"Frame");
	m_uniformBufferPools[2] = new UniformBufferPool(this, 100000, L"Draw");

	// Render passes are shared by all views.
	m_renderPassCache = new RenderPassCache(m_logicalDevice);

	// Bindless resources.
	const uint32_t bindings[] = { BindlessTexturesBinding, BindlessImagesBinding, BindlessBuffersBinding };
	VkDescriptorType descriptorTypes[] = { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER };
	VkDescriptorSetLayout* layouts[] = { &m_bindlessTexturesDescriptorLayout, &m_bindlessImagesDescriptorLayout, &m_bindlessBuffersDescriptorLayout };
	VkDescriptorSet* descriptorSets[] = { &m_bindlessTexturesDescriptorSet, &m_bindlessImagesDescriptorSet, &m_bindlessBuffersDescriptorSet };

	for (int32_t i = 0; i < sizeof_array(bindings); ++i)
	{
		const VkDescriptorBindingFlags bindlessFlags =
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
			VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT |
			VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT |
			VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT;

		const VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extendedInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT,
			.bindingCount = 1,
			.pBindingFlags = &bindlessFlags
		};

		const VkDescriptorSetLayoutBinding binding = {
			.binding = bindings[i],
			.descriptorType = descriptorTypes[i],
			.descriptorCount = MaxBindlessResources,
			.stageFlags = VK_SHADER_STAGE_ALL,
			.pImmutableSamplers = nullptr
		};

		const VkDescriptorSetLayoutCreateInfo layoutInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = &extendedInfo,
			.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,
			.bindingCount = 1,
			.pBindings = &binding
		};

		if (vkCreateDescriptorSetLayout(m_logicalDevice, &layoutInfo, nullptr, layouts[i]) != VK_SUCCESS)
		{
			log::error << L"Failed to create Vulkan; failed to create bindless descriptor layout." << Endl;
			return false;
		}

		// Create descriptor set; the variable count covers every index the allocators hand out.
		const uint32_t descriptorCount = MaxBindlessResources;

		const VkDescriptorSetVariableDescriptorCountAllocateInfoEXT countInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT,
			.descriptorSetCount = 1,
			.pDescriptorCounts = &descriptorCount
		};

		const VkDescriptorSetAllocateInfo allocInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = &countInfo,
			.descriptorPool = m_bindlessDescriptorPool,
			.descriptorSetCount = 1,
			.pSetLayouts = layouts[i]
		};

		VkResult result;
		if ((result = vkAllocateDescriptorSets(m_logicalDevice, &allocInfo, descriptorSets[i])) != VK_SUCCESS)
		{
			log::error << L"Failed to create Vulkan; failed to create bindless descriptor set. " << getHumanResult(result) << Endl;
			return false;
		}
	}

	return true;
}

void Context::incrementViews()
{
	Atomic::increment(m_views);
}

void Context::decrementViews()
{
	Atomic::decrement(m_views);
}

void Context::addDeferredCleanup(const cleanup_fn_t& fn, uint32_t cleanupFlags)
{
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_cleanupLock);

		// Nothing can record new work with a resource being destroyed; only wait for submissions made so far.
		m_cleanupFns.push_back({ fn, cleanupFlags, m_nextSubmissionEpoch - 1 });
	}
	if (m_views <= 0)
		performCleanupAll();
}

void Context::addCleanupListener(ICleanupListener* cleanupListener)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_cleanupLock);
	m_cleanupListeners.push_back(cleanupListener);
}

void Context::removeCleanupListener(ICleanupListener* cleanupListener)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_cleanupLock);
	auto it = std::find(m_cleanupListeners.begin(), m_cleanupListeners.end(), cleanupListener);
	if (it != m_cleanupListeners.end())
		m_cleanupListeners.erase(it);
}

void Context::performCleanup()
{
	// Read before taking the locks; submissions issued meanwhile only make this conservative, never premature.
	const uint64_t completedEpoch = getCompletedEpoch();

	{
		T_PROFILER_SCOPE(L"Context::performCleanup");

		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_graphicsQueue->m_lock);

		// Release upload command buffers the GPU is done with.
		retireUploads(false);

		if (m_cleanupFns.empty())
			return;

		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_computeQueue->m_lock);
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_cleanupLock);

		bool freeDescriptors = false;
		for (;;)
		{
			// Take over vector in case more resources are added for cleanup from callbacks.
			AlignedVector< DeferredCleanup > cleanupFns;
			cleanupFns.swap(m_cleanupFns);

			bool performed = false;
			for (const DeferredCleanup& cleanupFn : cleanupFns)
			{
				// Submissions which might still read the resource haven't been consumed; retain the cleanup.
				if (cleanupFn.waitEpoch > completedEpoch)
				{
					m_cleanupFns.push_back(cleanupFn);
					continue;
				}

				freeDescriptors |= (bool)((cleanupFn.flags & CleanupFreeDescriptorSets) != 0);
				cleanupFn.fn(this);
				performed = true;
			}

			if (!performed)
				break;
		}

		// Only call cleanup listeners to free descriptors.
		if (freeDescriptors)
			for (auto cleanupListener : m_cleanupListeners)
				cleanupListener->postCleanup();
	}
}

void Context::performCleanupAll()
{
	T_PROFILER_SCOPE(L"Context::performCleanupAll");

	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_graphicsQueue->m_lock);
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_computeQueue->m_lock);
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_cleanupLock);

	if (m_cleanupFns.empty() && m_uploadCommandBuffers.empty())
		return;

	// Every resource must be gone on return, so wait for the device instead of each cleanup's submissions.
	vkDeviceWaitIdle(m_logicalDevice);

	// Every submitted upload has been consumed now.
	retireUploads(true);

	// Device is idle; only a reserved submission still being recorded might use released resources.
	const uint64_t completedEpoch = getCompletedEpoch();

	bool freeDescriptors = false;
	for (;;)
	{
		for (;;)
		{
			// Take over vector in case more resources are added for cleanup from callbacks.
			AlignedVector< DeferredCleanup > cleanupFns;
			cleanupFns.swap(m_cleanupFns);

			bool performed = false;
			for (const DeferredCleanup& cleanupFn : cleanupFns)
			{
				if (cleanupFn.waitEpoch > completedEpoch)
				{
					m_cleanupFns.push_back(cleanupFn);
					continue;
				}

				freeDescriptors |= (bool)((cleanupFn.flags & CleanupFreeDescriptorSets) != 0);
				cleanupFn.fn(this);
				performed = true;
			}

			if (!performed)
				break;
		}

		// Only call cleanup listeners to free descriptors.
		if (!freeDescriptors)
			break;

		// Listeners may add cleanups of their own; drain those as well since every resource must be gone.
		freeDescriptors = false;
		for (auto cleanupListener : m_cleanupListeners)
			cleanupListener->postCleanup();
	}
}

uint64_t Context::beginSubmission(VkFence fence)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_submissionLock);
	const uint64_t epoch = m_nextSubmissionEpoch++;
	m_inFlightSubmissions.push_back({ epoch, fence, false });
	return epoch;
}

void Context::submissionIssued(uint64_t epoch)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_submissionLock);
	for (auto& submission : m_inFlightSubmissions)
	{
		if (submission.epoch == epoch)
		{
			submission.issued = true;
			break;
		}
	}
}

void Context::endSubmission(uint64_t epoch, VkFence fence)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_submissionLock);

	// Reset within the lock; until the submission is erased its fence may be polled from another thread.
	if (fence != VK_NULL_HANDLE)
		vkResetFences(m_logicalDevice, 1, &fence);

	for (auto it = m_inFlightSubmissions.begin(); it != m_inFlightSubmissions.end(); ++it)
	{
		if (it->epoch == epoch)
		{
			m_inFlightSubmissions.erase(it);
			break;
		}
	}
}

uint64_t Context::getCompletedEpoch()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_submissionLock);

	// Retire signalled submissions from the front; a command buffer never waited on mustn't hold cleanups back.
	while (!m_inFlightSubmissions.empty())
	{
		// A submission on its way into a queue has its fence owned by vkQueueSubmit; nothing is known yet.
		if (!m_inFlightSubmissions.front().issued)
			break;
		if (vkGetFenceStatus(m_logicalDevice, m_inFlightSubmissions.front().fence) != VK_SUCCESS)
			break;
		m_inFlightSubmissions.erase(m_inFlightSubmissions.begin());
	}

	// Epochs are appended in increasing order, so everything before the front submission has been consumed.
	if (!m_inFlightSubmissions.empty())
		return m_inFlightSubmissions.front().epoch - 1;
	else
		return m_nextSubmissionEpoch - 1;
}

void Context::waitForEpoch(uint64_t epoch)
{
	// Poll, as registered fences may be reset from other threads meanwhile.
	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
	while (getCompletedEpoch() < epoch)
		if (currentThread)
			currentThread->yield();
		else
			std::this_thread::yield();
}

void Context::frameSubmitted()
{
	T_ANONYMOUS_VAR(Acquire< CriticalSection >)(m_submittedFramesLock);
	const uint64_t frame = m_submittedFrames + 1;
	m_submittedFrameEpochs[frame % c_submittedFrameHistory] = getIssuedEpoch();
	m_submittedFrames = frame;
}

uint64_t Context::getSubmittedFrameEpoch(uint64_t frame) const
{
	T_ANONYMOUS_VAR(Acquire< CriticalSection >)(m_submittedFramesLock);
	const uint64_t submittedFrames = m_submittedFrames;
	T_ASSERT(frame >= 1 && frame <= submittedFrames);

	// Epochs only grow, so a later frame's epoch bounds an older one's from above.
	if (frame + c_submittedFrameHistory <= submittedFrames)
		frame = submittedFrames - c_submittedFrameHistory + 1;
	return m_submittedFrameEpochs[frame % c_submittedFrameHistory];
}

void Context::addDeferredUpload(const upload_fn_t& fn, uint32_t uploadSize)
{
	bool flush;
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_updateLock);
		m_uploadFns.push_back(fn);
		m_pendingUploadSize += uploadSize;
		flush = (m_pendingUploadSize >= c_maxPendingUploadSize);
	}

	// Without a view nothing ends frames; perform uploads right away and wait for them.
	if (m_views <= 0)
		performUploads(true);
	else if (flush)
		performUploads(false);
}

void Context::performUploads(bool wait)
{
	if (m_uploadFns.empty())
		return;

	T_PROFILER_SCOPE(L"Context::performUploads");

	Ref< CommandBuffer > commandBuffer;
	{
		// Only the graphics queue is held, and only while recording and submitting; not while uploads are consumed.
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_graphicsQueue->m_lock);

		// Release earlier upload command buffers the GPU is done with.
		retireUploads(false);

		// Grab the deferred upload queue.
		AlignedVector< upload_fn_t > uploadFns;
		uint32_t uploadSize;
		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_updateLock);
			uploadFns.swap(m_uploadFns);
			uploadSize = m_pendingUploadSize;
			m_pendingUploadSize = 0;
		}
		if (uploadFns.empty())
			return;

		// Create command buffer and execute the upload queue.
		commandBuffer = m_graphicsQueue->acquireCommandBuffer(L"Context::performUploads", m_uploadCommandPool);
		if (!commandBuffer)
		{
			// Failed to create command buffer; put back deferred uploads to queue.
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_updateLock);
			m_uploadFns.insert(m_uploadFns.begin(), uploadFns.begin(), uploadFns.end());
			m_pendingUploadSize += uploadSize;
			return;
		}

		// Uploads release staging resources as they are recorded; reserve first so those cleanups wait for the uploads.
		commandBuffer->reserveSubmission();

		for (const upload_fn_t& fn : uploadFns)
			fn(this, commandBuffer);

		// Make uploads available, and visible, to all work submitted later to this queue.
		const VkMemoryBarrier mb = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT
		};
		vkCmdPipelineBarrier(
			*commandBuffer,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			0,
			1,
			&mb,
			0,
			nullptr,
			0,
			nullptr);

		const uint64_t uploadValue = m_uploadValue + 1;
		if (!commandBuffer->submitSignal(m_uploadSemaphore, uploadValue))
			return;

		m_uploadValue = uploadValue;

		// Kept until consumed; released by a later flush or cleanup.
		if (!wait)
		{
			m_uploadCommandBuffers.push_back(commandBuffer);
			return;
		}
	}

	// Wait with the queue released; released with the queue held as it guards the upload pool.
	commandBuffer->wait();
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_graphicsQueue->m_lock);
		commandBuffer = nullptr;
	}

	// Without a view no frame performs cleanups; release what the uploads held back.
	if (m_views <= 0)
		performCleanupAll();
}

void Context::recycle()
{
	// Everything recorded so far has been submitted; blocks freed so far are reused once it is consumed.
	const uint64_t issuedEpoch = getIssuedEpoch();
	const uint64_t completedEpoch = getCompletedEpoch();
	for (int32_t i = 0; i < sizeof_array(m_uniformBufferPools); ++i)
		m_uniformBufferPools[i]->recycle(issuedEpoch, completedEpoch);
}

bool Context::savePipelineCache()
{
	size_t size = 0;
	vkGetPipelineCacheData(m_logicalDevice, m_pipelineCache, &size, nullptr);
	if (!size)
		return true;

	AlignedVector< uint8_t > buffer(size, 0);
	vkGetPipelineCacheData(m_logicalDevice, m_pipelineCache, &size, buffer.ptr());

	StringOutputStream ss;
#if defined(__IOS__)
	ss << OS::getInstance().getUserHomePath() << L"/Library/Caches/Traktor/Vulkan/Pipeline.cache";
#else
	ss << OS::getInstance().getWritableFolderPath() << L"/Traktor/Vulkan/Pipeline.cache";
#endif

	FileSystem::getInstance().makeAllDirectories(Path(ss.str()).getPathOnly());

	Ref< IStream > file = FileSystem::getInstance().open(ss.str(), File::FmWrite);
	if (!file)
	{
		log::error << L"Unable to save pipeline cache; failed to create file \"" << ss.str() << L"\"." << Endl;
		return false;
	}

	file->write(buffer.c_ptr(), size);
	file->close();

	log::debug << L"Pipeline cache \"" << ss.str() << L"\" saved successfully." << Endl;
	return true;
}

uint32_t Context::allocateSampledResourceIndex()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_resourceIndexLock);
	return m_sampledResourceIndexAllocator.alloc();
}

void Context::freeSampledResourceIndex(uint32_t resourceIndex)
{
	T_FATAL_ASSERT(resourceIndex != ~0U);
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_resourceIndexLock);
	m_sampledResourceIndexAllocator.free(resourceIndex);
}

uint32_t Context::allocateStorageResourceIndex(uint32_t span)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_resourceIndexLock);
	return m_storageResourceIndexAllocator.allocSequential(span);
}

void Context::freeStorageResourceIndex(uint32_t resourceIndex, uint32_t span)
{
	T_FATAL_ASSERT(resourceIndex != ~0U);
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_resourceIndexLock);
	m_storageResourceIndexAllocator.freeSequential(resourceIndex, span);
}

uint32_t Context::allocateBufferResourceIndex()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_resourceIndexLock);
	return m_bufferResourceIndexAllocator.alloc();
}

void Context::freeBufferResourceIndex(uint32_t resourceIndex)
{
	T_FATAL_ASSERT(resourceIndex != ~0U);
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_resourceIndexLock);
	m_bufferResourceIndexAllocator.free(resourceIndex);
}

void Context::updateBindlessDescriptors(const VkWriteDescriptorSet* writes, uint32_t writeCount)
{
	T_ANONYMOUS_VAR(Acquire< CriticalSection >)(m_bindlessLock);
	vkUpdateDescriptorSets(m_logicalDevice, writeCount, writes, 0, nullptr);
}

VkPipeline Context::validateGraphicsPipeline(const VertexLayoutVk* vertexLayout, const ProgramVk* program, PrimitiveType pt, const RenderTargetSetVk* targetSet, VkRenderPass targetRenderPass, float multiSampleShading)
{
	// Calculate pipeline key; render passes live as long as pipelines, so the handle identifies them.
	const uint8_t primitiveId = (uint8_t)pt;
	const uint32_t declHash = (vertexLayout != nullptr) ? vertexLayout->getHash() : 0;
	const uint32_t shaderHash = program->getShaderHash();
	const auto key = std::make_tuple(primitiveId, (uint64_t)targetRenderPass, declHash, shaderHash);

	// Pipelines are created, and looked up, from multiple threads.
	T_ANONYMOUS_VAR(Acquire< CriticalSection >)(m_pipelinesLock);

	auto it = m_pipelines.find(key);
	if (it != m_pipelines.end())
		return it->second;

	VkPipeline pipeline = 0;

	const RenderState& rs = program->getRenderState();
	const uint32_t colorAttachmentCount = targetSet->getColorTargetCount();

	const VkViewport vp = {
		.width = 1,
		.height = 1,
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};

	const VkRect2D sc = {
		.offset = { 0, 0 },
		.extent = { 65536, 65536 }
	};

	const VkPipelineViewportStateCreateInfo vsci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports = &vp,
		.scissorCount = 1,
		.pScissors = &sc
	};

	VkPipelineVertexInputStateCreateInfo visci = {};
	visci.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	if (vertexLayout != nullptr)
	{
		visci.vertexBindingDescriptionCount = 1;
		visci.pVertexBindingDescriptions = &vertexLayout->getVkVertexInputBindingDescription();
		visci.vertexAttributeDescriptionCount = (uint32_t)vertexLayout->getVkVertexInputAttributeDescriptions().size();
		visci.pVertexAttributeDescriptions = vertexLayout->getVkVertexInputAttributeDescriptions().c_ptr();
	}
	else
	{
		visci.vertexBindingDescriptionCount = 0;
		visci.pVertexBindingDescriptions = nullptr;
		visci.vertexAttributeDescriptionCount = 0;
		visci.pVertexAttributeDescriptions = nullptr;
	}

	StaticVector< VkPipelineShaderStageCreateInfo, 2 > ssci;
	ssci.push_back({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = program->getVertexVkShaderModule(),
		.pName = "main",
		.pSpecializationInfo = nullptr });
	ssci.push_back({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = program->getFragmentVkShaderModule(),
		.pName = "main",
		.pSpecializationInfo = nullptr });

	const bool isLineTopology = (pt == PrimitiveType::Lines || pt == PrimitiveType::LineStrip);

	const VkPipelineRasterizationLineStateCreateInfoKHR lineStateCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO_KHR,
		.pNext = nullptr,
		.lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_KHR,
		.stippledLineEnable = VK_FALSE,
		.lineStippleFactor = 0,
		.lineStipplePattern = 0
	};

	const VkPipelineRasterizationStateCreateInfo rsci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.pNext = (m_smoothLines && isLineTopology) ? (const void*)&lineStateCreateInfo : nullptr,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = rs.wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL,
		.cullMode = (VkCullModeFlags)c_cullMode[(int32_t)rs.cullMode],
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.depthBiasConstantFactor = 0,
		.depthBiasClamp = 0,
		.depthBiasSlopeFactor = 0,
		.lineWidth = 1
	};

	VkPipelineMultisampleStateCreateInfo mssci = {};
	mssci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	mssci.rasterizationSamples = targetSet->getVkSampleCount();
	if (!isLineTopology && multiSampleShading > FUZZY_EPSILON)
	{
		mssci.sampleShadingEnable = VK_TRUE;
		mssci.minSampleShading = multiSampleShading;
	}
	else
		mssci.sampleShadingEnable = VK_FALSE;
	mssci.pSampleMask = nullptr;
	mssci.alphaToCoverageEnable = rs.alphaToCoverageEnable ? VK_TRUE : VK_FALSE;
	mssci.alphaToOneEnable = VK_FALSE;

	const VkStencilOpState sops = {
		.failOp = c_stencilOperations[(int)rs.stencilFail],
		.passOp = c_stencilOperations[(int)rs.stencilPass],
		.depthFailOp = c_stencilOperations[(int)rs.stencilZFail],
		.compareOp = c_compareOperations[(int)rs.stencilFunction],
		.compareMask = (uint32_t)~0U, //rs.stencilMask,
		.writeMask = (uint32_t)~0U, //rs.stencilMask,
		.reference = rs.stencilReference
	};

	const VkPipelineDepthStencilStateCreateInfo dssci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = rs.depthEnable ? VK_TRUE : VK_FALSE,
		.depthWriteEnable = rs.depthWriteEnable ? VK_TRUE : VK_FALSE,
		.depthCompareOp = rs.depthEnable ? c_compareOperations[(int)rs.depthFunction] : VK_COMPARE_OP_ALWAYS,
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable = rs.stencilEnable ? VK_TRUE : VK_FALSE,
		.front = sops,
		.back = sops,
		.minDepthBounds = 0,
		.maxDepthBounds = 0
	};

	StaticVector< VkPipelineColorBlendAttachmentState, RenderTargetSetCreateDesc::MaxTargets > blendAttachments;
	for (uint32_t i = 0; i < colorAttachmentCount; ++i)
	{
		auto& cbas = blendAttachments.push_back();
		cbas.blendEnable = rs.blendEnable ? VK_TRUE : VK_FALSE;
		cbas.srcColorBlendFactor = c_blendFactors[(int)rs.blendColorSource];
		cbas.dstColorBlendFactor = c_blendFactors[(int)rs.blendColorDestination];
		cbas.colorBlendOp = c_blendOperations[(int)rs.blendColorOperation];
		cbas.srcAlphaBlendFactor = c_blendFactors[(int)rs.blendAlphaSource];
		cbas.dstAlphaBlendFactor = c_blendFactors[(int)rs.blendAlphaDestination];
		cbas.alphaBlendOp = c_blendOperations[(int)rs.blendAlphaOperation];
		cbas.colorWriteMask = rs.colorWriteMask;
	}

	const VkPipelineColorBlendStateCreateInfo cbsci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_CLEAR,
		.attachmentCount = (uint32_t)blendAttachments.size(),
		.pAttachments = blendAttachments.c_ptr(),
		.blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f }
	};

	const VkDynamicState ds[3] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_STENCIL_REFERENCE };
	const VkPipelineDynamicStateCreateInfo dsci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = rs.stencilEnable ? 3U : 2U,
		.pDynamicStates = ds
	};

	const VkPipelineInputAssemblyStateCreateInfo iasci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = c_primitiveTopology[(int32_t)pt],
		.primitiveRestartEnable = VK_FALSE
	};

	const VkGraphicsPipelineCreateInfo gpci = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = (uint32_t)ssci.size(),
		.pStages = ssci.c_ptr(),
		.pVertexInputState = &visci,
		.pInputAssemblyState = &iasci,
		.pTessellationState = nullptr,
		.pViewportState = &vsci,
		.pRasterizationState = &rsci,
		.pMultisampleState = &mssci,
		.pDepthStencilState = &dssci,
		.pColorBlendState = &cbsci,
		.pDynamicState = &dsci,
		.layout = program->getPipelineLayout(),
		.renderPass = targetRenderPass,
		.subpass = 0,
		.basePipelineHandle = 0,
		.basePipelineIndex = 0
	};

	const VkResult result = vkCreateGraphicsPipelines(
		m_logicalDevice,
		m_pipelineCache,
		1,
		&gpci,
		nullptr,
		&pipeline);
	if (result != VK_SUCCESS)
	{
#if defined(_DEBUG)
		log::error << L"Unable to create Vulkan graphics pipeline (" << getHumanResult(result) << L"), \"" << program->getTag() << L"\"." << Endl;
#else
		log::error << L"Unable to create Vulkan graphics pipeline (" << getHumanResult(result) << L")." << Endl;
#endif
		return 0;
	}

	m_pipelines.insert(key, pipeline);
#if defined(_DEBUG)
	log::debug << L"Graphics pipeline created (" << program->getTag() << L", " << m_pipelines.size() << L" pipelines)." << Endl;
#endif
	return pipeline;
}

VkPipeline Context::validateComputePipeline(const ProgramVk* p)
{
	const uint32_t shaderHash = p->getShaderHash();

	// Created, and looked up, from multiple threads.
	T_ANONYMOUS_VAR(Acquire< CriticalSection >)(m_pipelinesLock);

	auto it = m_computePipelines.find(shaderHash);
	if (it != m_computePipelines.end())
		return it->second;

	VkPipeline pipeline = 0;

	const VkPipelineShaderStageCreateInfo ssci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_COMPUTE_BIT,
		.module = p->getComputeVkShaderModule(),
		.pName = "main",
		.pSpecializationInfo = nullptr
	};

	const VkComputePipelineCreateInfo cpci = {
		.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
		.stage = ssci,
		.layout = p->getPipelineLayout()
	};

	const VkResult result = vkCreateComputePipelines(
		m_logicalDevice,
		m_pipelineCache,
		1,
		&cpci,
		nullptr,
		&pipeline);
	if (result != VK_SUCCESS)
	{
#if defined(_DEBUG)
		log::error << L"Unable to create Vulkan compute pipeline (" << getHumanResult(result) << L"), \"" << p->getTag() << L"\"." << Endl;
#else
		log::error << L"Unable to create Vulkan compute pipeline (" << getHumanResult(result) << L")." << Endl;
#endif
		return 0;
	}

	m_computePipelines.insert(shaderHash, pipeline);
#if defined(_DEBUG)
	log::debug << L"Compute pipeline created (" << p->getTag() << L", " << m_computePipelines.size() << L" pipelines)." << Endl;
#endif
	return pipeline;
}

void Context::setObjectDebugName(const wchar_t* const tag, uint64_t object, VkObjectType objectType)
{
#if !defined(__ANDROID__) && !defined(__APPLE__)
	static CriticalSection s_debugNameLock;
	T_ANONYMOUS_VAR(Acquire< CriticalSection >)(s_debugNameLock);

	static SmallMap< VkObjectType, uint32_t > s_objectCount;
	uint32_t& count = s_objectCount[objectType];

	std::stringstream ss;
	if (tag)
		ss << wstombs(tag) << " [" << count << "]";
	else
		ss << "<unnamed> [" << count << "]";
	++count;

	// The name is copied by the implementation; it doesn't have to outlive the call.
	const std::string name = ss.str();

	const VkDebugUtilsObjectNameInfoEXT ni = {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
		.objectType = objectType,
		.objectHandle = object,
		.pObjectName = name.c_str()
	};
	vkSetDebugUtilsObjectNameEXT(m_logicalDevice, &ni);
#endif
}


void Context::retireUploads(bool all)
{
	if (m_uploadCommandBuffers.empty())
		return;

	const uint64_t completedEpoch = all ? ~0ULL : getCompletedEpoch();
	for (auto it = m_uploadCommandBuffers.begin(); it != m_uploadCommandBuffers.end();)
	{
		CommandBuffer* commandBuffer = *it;
		if (commandBuffer->getSubmissionEpoch() <= completedEpoch)
		{
			// Fence has signalled, or the device is idle, thus this doesn't block.
			if (all)
				commandBuffer->externalSynced();
			else
				commandBuffer->wait();
			it = m_uploadCommandBuffers.erase(it);
		}
		else
			++it;
	}
}

}
