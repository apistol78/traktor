/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/IdAllocator.h"
#include "Core/Thread/Semaphore.h"
#include "Render/Vulkan/Private/ApiHeader.h"

namespace traktor::render
{

class Queue;
class UniformBufferPool;

/*! Render system shared context.
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
	constexpr static uint32_t CleanupNeedFlushGPU = 1;

	typedef std::function< void(Context*) > cleanup_fn_t;

	struct ICleanupListener
	{
		virtual void postCleanup() = 0;
	};

	explicit Context(
		VkPhysicalDevice physicalDevice,
		VkDevice logicalDevice,
		VmaAllocator allocator,
		uint32_t graphicsQueueIndex,
		uint32_t computeQueueIndex
	);

	virtual ~Context();

	bool create();

	void incrementViews();

	void decrementViews();

	/*! Add a deferred cleanup.
	 *
	 * Deferred cleanups are issued after the current
	 * frame has finished, from the calling thread
	 * of present.
	 */
	void addDeferredCleanup(const cleanup_fn_t& fn, uint32_t cleanupFlags);

	void addCleanupListener(ICleanupListener* cleanupListener);

	void removeCleanupListener(ICleanupListener* cleanupListener);

	void performCleanup();

	void recycle();

	bool savePipelineCache();

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

private:
	struct DeferredCleanup
	{
		cleanup_fn_t fn;
		uint32_t flags;
	};

	VkPhysicalDevice m_physicalDevice;
	VkDevice m_logicalDevice;
	VmaAllocator m_allocator;
	uint32_t m_graphicsQueueIndex;
	uint32_t m_computeQueueIndex;
	VkPipelineCache m_pipelineCache = 0;
	VkDescriptorPool m_descriptorPool = 0;
	int32_t m_views = 0;
	Ref< Queue > m_graphicsQueue;
	Ref< Queue > m_computeQueue;
	Ref< UniformBufferPool > m_uniformBufferPools[3];
	Semaphore m_cleanupLock;
	Semaphore m_resourceIndexLock;
	AlignedVector< DeferredCleanup > m_cleanupFns;
	AlignedVector< ICleanupListener* > m_cleanupListeners;
	VkDescriptorSetLayout m_bindlessTexturesDescriptorLayout = 0;
	VkDescriptorSet m_bindlessTexturesDescriptorSet = 0;
	VkDescriptorSetLayout m_bindlessImagesDescriptorLayout = 0;
	VkDescriptorSet m_bindlessImagesDescriptorSet = 0;
	VkDescriptorSetLayout m_bindlessBuffersDescriptorLayout = 0;
	VkDescriptorSet m_bindlessBuffersDescriptorSet = 0;
	IdAllocator m_sampledResourceIndexAllocator;
	IdAllocator m_storageResourceIndexAllocator;
	IdAllocator m_bufferResourceIndexAllocator;
};

}
