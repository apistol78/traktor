/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <atomic>
#include <functional>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/IdAllocator.h"
#include "Core/Thread/Semaphore.h"
#include "Render/Types.h"
#include "Render/Vulkan/Private/ApiHeader.h"

namespace traktor::render
{

class CommandBuffer;
class ProgramVk;
class RenderTargetSetVk;
class Queue;
class UniformBufferPool;
class VertexLayoutVk;

/*! Render system shared context.
 * \ingroup Render
 *
 * This context is owned by render system and shared
 * across all render views.
 */
class Context : public Object
{
	T_RTTI_CLASS;

public:
	constexpr static uint32_t MaxBindlessResources = 16536;
	constexpr static uint32_t BindlessTexturesBinding = 0;
	constexpr static uint32_t BindlessImagesBinding = 1;
	constexpr static uint32_t BindlessBuffersBinding = 2;
	constexpr static uint32_t NonBindlessFirstBinding = 3;

	constexpr static uint32_t CleanupNone = 0;
	constexpr static uint32_t CleanupFreeDescriptorSets = 1;

	typedef std::function< void(Context*) > cleanup_fn_t;
	typedef std::function< void(Context*, CommandBuffer*) > upload_fn_t;

	struct ICleanupListener
	{
		virtual void postCleanup() = 0;
	};

	explicit Context(
		VkInstance instance,
		VkPhysicalDevice physicalDevice,
		VkDevice logicalDevice,
		VmaAllocator allocator,
		uint32_t graphicsQueueIndex,
		uint32_t computeQueueIndex,
		bool rayTracing,
		bool smoothLines
	);

	virtual ~Context();

	bool create();

	void incrementViews();

	void decrementViews();

	/*! Add a deferred cleanup.
	 *
	 * Deferred cleanups are issued once every submission which was in flight when
	 * the cleanup was added has been consumed by the GPU, from the calling thread
	 * of present.
	 */
	void addDeferredCleanup(const cleanup_fn_t& fn, uint32_t cleanupFlags);

	void addCleanupListener(ICleanupListener* cleanupListener);

	void removeCleanupListener(ICleanupListener* cleanupListener);

	/*! Perform those cleanups whose submissions have been consumed by the GPU. */
	void performCleanup();

	/*! Perform every pending cleanup; waits for the device to go idle first.
	 *
	 * Only for tear down paths, such as closing or resetting a view, where every
	 * resource has to be gone when the call returns.
	 */
	void performCleanupAll();

	/*! \name Submission tracking.
	 *
	 * Every command buffer submission is tagged with a monotonically increasing
	 * epoch, which lets deferred cleanups tell whether the submissions that could
	 * still be reading their resource have been consumed by the GPU.
	 *
	 * A registered fence is polled from any thread performing cleanups, not only
	 * from the thread owning the command buffer, so the submission lock doubles as
	 * the external synchronization of host access to those fences; every host
	 * operation on a registered fence must be issued through these methods.
	 */
	//@{

	/*! Register a submission about to be issued; \sa submissionIssued, \sa endSubmission.
	 *
	 * The fence is not polled until the submission has been reported as issued, as
	 * handing it to vkQueueSubmit is itself a host access to the fence.
	 *
	 * \param fence Fence signalled when the submission has been consumed.
	 */
	uint64_t beginSubmission(VkFence fence);

	/*! Register a submission as issued to its queue, thus its fence can be polled. */
	void submissionIssued(uint64_t epoch);

	/*! Register a submission as consumed by the GPU; also resets its fence.
	 *
	 * \param epoch Submission epoch, from beginSubmission.
	 * \param fence Fence to reset, or VK_NULL_HANDLE to leave it as-is.
	 */
	void endSubmission(uint64_t epoch, VkFence fence);

	/*! Get the epoch up until, and including, which all submissions are consumed. */
	uint64_t getCompletedEpoch();

	//@}

	/*! Add a deferred upload.
	 *
	 * Uploads are recorded into a single command buffer, and performed, by the
	 * thread which ends a frame; \sa performUploads. Resources creating an upload
	 * must keep their staging memory alive until it has been performed, thus the
	 * queue is flushed early when too much is pending.
	 *
	 * \param fn Records the upload into the flush's command buffer.
	 * \param uploadSize Amount of staging memory held back by this upload, in bytes.
	 */
	void addDeferredUpload(const upload_fn_t& fn, uint32_t uploadSize = 0);

	void performUploads();

	void recycle();

	bool savePipelineCache();

	VkInstance getInstance() const { return m_instance; }

	VkPhysicalDevice getPhysicalDevice() const { return m_physicalDevice; }

	VkDevice getLogicalDevice() const { return m_logicalDevice; }

	VmaAllocator getAllocator() const { return m_allocator; }

	VkPipelineCache getPipelineCache() const { return m_pipelineCache; }

	VkDescriptorPool getDescriptorPool() const { return m_descriptorPool; }

	Queue* getGraphicsQueue() const { return m_graphicsQueue; }

	Queue* getComputeQueue() const { return m_computeQueue; }

	UniformBufferPool* getUniformBufferPool(int32_t index) const { return m_uniformBufferPools[index]; }

	VkDescriptorSetLayout getBindlessTexturesSetLayout() const { return m_bindlessTexturesDescriptorLayout; }

	VkDescriptorSet getBindlessTexturesDescriptorSet() const { return m_bindlessTexturesDescriptorSet; }

	VkDescriptorSetLayout getBindlessImagesSetLayout() const { return m_bindlessImagesDescriptorLayout; }

	VkDescriptorSet getBindlessImagesDescriptorSet() const { return m_bindlessImagesDescriptorSet; }

	VkDescriptorSetLayout getBindlessBuffersSetLayout() const { return m_bindlessBuffersDescriptorLayout; }

	VkDescriptorSet getBindlessBuffersDescriptorSet() const { return m_bindlessBuffersDescriptorSet; }

	uint32_t allocateSampledResourceIndex();

	void freeSampledResourceIndex(uint32_t resourceIndex);

	uint32_t allocateStorageResourceIndex(uint32_t span);

	void freeStorageResourceIndex(uint32_t resourceIndex, uint32_t span);

	uint32_t allocateBufferResourceIndex();

	void freeBufferResourceIndex(uint32_t resourceIndex);

	VkPipeline validateGraphicsPipeline(const VertexLayoutVk* vertexLayout, const ProgramVk* program, PrimitiveType pt, uint32_t targetRenderPassHash, const RenderTargetSetVk* targetSet, VkRenderPass targetRenderPass, float multiSampleShading);

	VkPipeline validateComputePipeline(const ProgramVk* p);

	void setObjectDebugName(const wchar_t* const tag, uint64_t object, VkObjectType objectType);

private:
	struct DeferredCleanup
	{
		cleanup_fn_t fn;
		uint32_t flags;
		uint64_t waitEpoch;	//!< Cleanup is performed once every submission up until this epoch has been consumed.
	};

	struct Submission
	{
		uint64_t epoch;
		VkFence fence;
		bool issued;	//!< Submission has been handed to its queue, thus fence can be polled.
	};

	struct PipelineEntry
	{
		uint32_t lastAcquired;
		VkPipeline pipeline;
	};

	typedef std::tuple< uint8_t, uint32_t, uint32_t, uint32_t > pipeline_key_t;

	VkInstance m_instance;
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_logicalDevice;
	VmaAllocator m_allocator;
	uint32_t m_graphicsQueueIndex;
	uint32_t m_computeQueueIndex;
	bool m_rayTracing = false;
	bool m_smoothLines = false;
	VkPipelineCache m_pipelineCache = 0;
	VkDescriptorPool m_descriptorPool = 0;
	int32_t m_views = 0;
	Ref< Queue > m_graphicsQueue;
	Ref< Queue > m_computeQueue;
	Ref< UniformBufferPool > m_uniformBufferPools[3];
	Semaphore m_cleanupLock;
	Semaphore m_updateLock;
	Semaphore m_resourceIndexLock;
	Semaphore m_submissionLock;
	std::atomic< uint64_t > m_nextSubmissionEpoch = 1;
	AlignedVector< Submission > m_inFlightSubmissions;	//!< Submissions not known to be consumed, in increasing epoch order.
	AlignedVector< DeferredCleanup > m_cleanupFns;
	AlignedVector< ICleanupListener* > m_cleanupListeners;
	AlignedVector< upload_fn_t > m_uploadFns;
	uint32_t m_pendingUploadSize = 0;	//!< Staging memory held back by queued uploads.
	VkDescriptorSetLayout m_bindlessTexturesDescriptorLayout = 0;
	VkDescriptorSet m_bindlessTexturesDescriptorSet = 0;
	VkDescriptorSetLayout m_bindlessImagesDescriptorLayout = 0;
	VkDescriptorSet m_bindlessImagesDescriptorSet = 0;
	VkDescriptorSetLayout m_bindlessBuffersDescriptorLayout = 0;
	VkDescriptorSet m_bindlessBuffersDescriptorSet = 0;
	IdAllocator m_sampledResourceIndexAllocator;
	IdAllocator m_storageResourceIndexAllocator;
	IdAllocator m_bufferResourceIndexAllocator;
	SmallMap< pipeline_key_t, PipelineEntry > m_pipelines;

#if !defined(__ANDROID__) && !defined(__APPLE__)
	AlignedVector< char* > m_debugNames;
#endif
};

}
