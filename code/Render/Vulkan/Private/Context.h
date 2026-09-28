/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <atomic>
#include <functional>
#include <tuple>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/IdAllocator.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Thread/CriticalSection.h"
#include "Core/Thread/Semaphore.h"
#include "Render/Types.h"
#include "Render/Vulkan/Private/ApiHeader.h"

namespace traktor::render
{

class CommandBuffer;
class ProgramVk;
class RenderPassCache;
class RenderTargetSetVk;
class Queue;
class UniformBufferPool;
class VertexLayoutVk;

/*! Render system context, shared by all render views.
 * \ingroup Render
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
		bool smoothLines,
		bool hostQueryReset
	);

	virtual ~Context();

	bool create();

	void incrementViews();

	void decrementViews();

	/*! Add a deferred cleanup, performed once every submission in flight when added has been consumed by the GPU. */
	void addDeferredCleanup(const cleanup_fn_t& fn, uint32_t cleanupFlags);

	void addCleanupListener(ICleanupListener* cleanupListener);

	void removeCleanupListener(ICleanupListener* cleanupListener);

	/*! Perform those cleanups whose submissions have been consumed by the GPU. */
	void performCleanup();

	/*! Wait for the device to go idle, then perform every pending cleanup so all resources are gone on return. */
	void performCleanupAll();

	/*! \name Submission tracking.
	 * Submissions get increasing epochs; their fences are polled from any thread, thus only reset through these methods.
	 */
	//@{

	/*! Register a submission about to be issued and return its epoch; fence signals once it has been consumed.
	 * The fence isn't polled until the submission is reported issued, as vkQueueSubmit accesses it.
	 */
	uint64_t beginSubmission(VkFence fence);

	/*! Register a submission as issued to its queue, thus its fence can be polled. */
	void submissionIssued(uint64_t epoch);

	/*! Register a submission as consumed by the GPU and reset its fence, unless fence is VK_NULL_HANDLE. */
	void endSubmission(uint64_t epoch, VkFence fence);

	/*! Get the epoch up until, and including, which all submissions are consumed. */
	uint64_t getCompletedEpoch();

	/*! Get the most recently handed out epoch; submissions made so far are all at or below it. */
	uint64_t getIssuedEpoch() const { return m_nextSubmissionEpoch - 1; }

	/*! Block the calling thread until every submission up until, and including, epoch has been consumed. */
	void waitForEpoch(uint64_t epoch);

	/*! Register that a view has submitted all work of a frame. */
	void frameSubmitted();

	/*! Get number of frames submitted so far, by all views. */
	uint64_t getSubmittedFrameCount() const { return m_submittedFrames; }

	/*! Get epoch issued when frame, numbered from 1, was submitted; frames too old get a later epoch. */
	uint64_t getSubmittedFrameEpoch(uint64_t frame) const;

	//@}

	/*! Add a deferred upload, recorded by fn when flushed; uploadSize is the staging memory it holds back, in bytes. */
	void addDeferredUpload(const upload_fn_t& fn, uint32_t uploadSize = 0);

	/*! Record and submit queued uploads to the graphics queue, ordered before later work on it.
	 * The queue is only held while recording and submitting; wait also waits until the uploads have been consumed.
	 */
	void performUploads(bool wait = false);

	/*! Timeline semaphore signalled with the value of each upload submission. */
	VkSemaphore getUploadSemaphore() const { return m_uploadSemaphore; }

	/*! Value of last upload submission; must be read with graphics queue held. */
	uint64_t getUploadValue() const { return m_uploadValue; }

	/*! Return uniform buffer blocks whose last use has been consumed by the GPU. */
	void recycle();

	bool savePipelineCache();

	VkInstance getInstance() const { return m_instance; }

	VkPhysicalDevice getPhysicalDevice() const { return m_physicalDevice; }

	VkDevice getLogicalDevice() const { return m_logicalDevice; }

	VmaAllocator getAllocator() const { return m_allocator; }

	VkPipelineCache getPipelineCache() const { return m_pipelineCache; }

	/*! Pool of per-program descriptor sets. */
	VkDescriptorPool getDescriptorPool() const { return m_descriptorPool; }

	Queue* getGraphicsQueue() const { return m_graphicsQueue; }

	Queue* getComputeQueue() const { return m_computeQueue; }

	/*! Queries can be reset from the host, vkResetQueryPool. */
	bool haveHostQueryReset() const { return m_hostQueryReset; }

	UniformBufferPool* getUniformBufferPool(int32_t index) const { return m_uniformBufferPools[index]; }

	/*! Render passes, shared by every view. */
	RenderPassCache* getRenderPassCache() const { return m_renderPassCache; }

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

	/*! Write descriptors into the shared bindless sets; serialized since resources are created on any thread. */
	void updateBindlessDescriptors(const VkWriteDescriptorSet* writes, uint32_t writeCount);

	VkPipeline validateGraphicsPipeline(const VertexLayoutVk* vertexLayout, const ProgramVk* program, PrimitiveType pt, const RenderTargetSetVk* targetSet, VkRenderPass targetRenderPass, float multiSampleShading);

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

	//! Primitive type, render pass, vertex layout hash and shader hash.
	typedef std::tuple< uint8_t, uint64_t, uint32_t, uint32_t > pipeline_key_t;

	VkInstance m_instance;
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_logicalDevice;
	VmaAllocator m_allocator;
	uint32_t m_graphicsQueueIndex;
	uint32_t m_computeQueueIndex;
	bool m_rayTracing = false;
	bool m_smoothLines = false;
	bool m_hostQueryReset = false;
	VkPipelineCache m_pipelineCache = 0;
	VkDescriptorPool m_descriptorPool = 0;
	VkDescriptorPool m_bindlessDescriptorPool = 0;
	int32_t m_views = 0;
	Ref< Queue > m_graphicsQueue;
	Ref< Queue > m_computeQueue;
	Ref< UniformBufferPool > m_uniformBufferPools[3];
	Ref< RenderPassCache > m_renderPassCache;
	Semaphore m_cleanupLock;
	Semaphore m_updateLock;
	Semaphore m_resourceIndexLock;
	Semaphore m_submissionLock;
	CriticalSection m_pipelinesLock;
	CriticalSection m_bindlessLock;
	std::atomic< uint64_t > m_nextSubmissionEpoch = 1;
	AlignedVector< Submission > m_inFlightSubmissions;	//!< Submissions not known to be consumed, in increasing epoch order.
	static constexpr uint32_t c_submittedFrameHistory = 64;
	mutable CriticalSection m_submittedFramesLock;
	std::atomic< uint64_t > m_submittedFrames = 0;	//!< Number of frames submitted by all views.
	uint64_t m_submittedFrameEpochs[c_submittedFrameHistory] = {};	//!< Issued epoch when frame n was submitted, at n modulo history; guarded by m_submittedFramesLock.
	AlignedVector< DeferredCleanup > m_cleanupFns;
	AlignedVector< ICleanupListener* > m_cleanupListeners;
	AlignedVector< upload_fn_t > m_uploadFns;
	uint32_t m_pendingUploadSize = 0;
	VkSemaphore m_uploadSemaphore = VK_NULL_HANDLE;
	uint64_t m_uploadValue = 0;	//!< Value of last upload submission; guarded by graphics queue lock.
	VkCommandPool m_uploadCommandPool = 0;	//!< Pool of upload command buffers; guarded by graphics queue lock.
	AlignedVector< Ref< CommandBuffer > > m_uploadCommandBuffers;	//!< Submitted upload command buffers not yet known to be consumed; guarded by graphics queue lock.
	VkDescriptorSetLayout m_bindlessTexturesDescriptorLayout = 0;
	VkDescriptorSet m_bindlessTexturesDescriptorSet = 0;
	VkDescriptorSetLayout m_bindlessImagesDescriptorLayout = 0;
	VkDescriptorSet m_bindlessImagesDescriptorSet = 0;
	VkDescriptorSetLayout m_bindlessBuffersDescriptorLayout = 0;
	VkDescriptorSet m_bindlessBuffersDescriptorSet = 0;
	IdAllocator m_sampledResourceIndexAllocator;
	IdAllocator m_storageResourceIndexAllocator;
	IdAllocator m_bufferResourceIndexAllocator;
	SmallMap< pipeline_key_t, VkPipeline > m_pipelines;	//!< Graphics pipelines; guarded by pipelines lock.
	SmallMap< uint32_t, VkPipeline > m_computePipelines;	//!< Compute pipelines by shader hash; guarded by pipelines lock.

	/*! Release consumed upload command buffers, or all when the device is idle; caller holds the graphics queue. */
	void retireUploads(bool all);
};

}
