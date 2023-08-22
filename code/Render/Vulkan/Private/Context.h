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
	typedef std::function< void(Context*) > cleanup_fn_t;

	struct ICleanupListener
	{
		virtual void postCleanup() = 0;
	};

	explicit Context(
		VkPhysicalDevice physicalDevice,
		VkDevice logicalDevice,
		VmaAllocator allocator,
		uint32_t graphicsQueueIndex
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
	void addDeferredCleanup(const cleanup_fn_t& fn);

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

	//uint32_t getDescriptorPoolRevision() const { return m_descriptorPoolRevision; }

	Queue* getGraphicsQueue() const { return m_graphicsQueue; }

	UniformBufferPool* getUniformBufferPool(int32_t index) const { return m_uniformBufferPools[index]; }


	VkDescriptorSetLayout getBindlessSetLayout() const { return m_bindlessDescriptorLayout; }

	VkDescriptorSet getBindlessDescriptorSet() const { return m_bindlessDescriptorSet; }

	uint32_t allocBindlessResourceIndex();

	void freeResourceIndex(uint32_t &resourceIndex);

private:
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_logicalDevice;
	VmaAllocator m_allocator;
	uint32_t m_graphicsQueueIndex;
	VkPipelineCache m_pipelineCache;
	VkDescriptorPool m_descriptorPool;
	int32_t m_views;
	//uint32_t m_descriptorPoolRevision;
	Ref< Queue > m_graphicsQueue;
	Ref< UniformBufferPool > m_uniformBufferPools[3];
	Semaphore m_cleanupLock;
	AlignedVector< cleanup_fn_t > m_cleanupFns;
	AlignedVector< ICleanupListener* > m_cleanupListeners;

	VkDescriptorSetLayout m_bindlessDescriptorLayout;
	VkDescriptorSet m_bindlessDescriptorSet;

	IdAllocator m_resourceIndexAllocator;
};

}
