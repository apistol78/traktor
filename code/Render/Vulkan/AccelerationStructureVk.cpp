/*
 * TRAKTOR
 * Copyright (c) 2024-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Vulkan/AccelerationStructureVk.h"

#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/Buffer.h"
#include "Render/Vulkan/BufferStaticVk.h"
#include "Render/Vulkan/BufferViewVk.h"
#include "Render/Vulkan/Private/ApiBuffer.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/Queue.h"
#include "Render/Vulkan/VertexLayoutVk.h"

namespace traktor::render
{
namespace
{

uint32_t getScratchAlignment(Context* context)
{
	VkPhysicalDeviceAccelerationStructurePropertiesKHR asp = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR
	};
	VkPhysicalDeviceProperties2 deviceProperties = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
		.pNext = &asp
	};
	vkGetPhysicalDeviceProperties2(context->getPhysicalDevice(), &deviceProperties);
	return std::max< uint32_t >(128, asp.minAccelerationStructureScratchOffsetAlignment);
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.AccelerationStructureVk", AccelerationStructureVk, IAccelerationStructure)

AccelerationStructureVk::~AccelerationStructureVk()
{
	teardown();
}

Ref< AccelerationStructureVk > AccelerationStructureVk::createTopLevel(Context* context, uint32_t numInstances, bool pooled)
{
	// Addresses of the build data are not used when only querying sizes.
	VkAccelerationStructureGeometryDataKHR topLevelAccelerationStructureGeometryData = {
		.instances = {
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
			.pNext = nullptr,
			.arrayOfPointers = VK_FALSE,
			.data = {
				.deviceAddress = 0 } }
	};

	VkAccelerationStructureGeometryKHR topLevelAccelerationStructureGeometry = {
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
		.pNext = nullptr,
		.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
		.geometry = topLevelAccelerationStructureGeometryData,
		.flags = VK_GEOMETRY_OPAQUE_BIT_KHR
	};

	VkAccelerationStructureBuildGeometryInfoKHR topLevelAccelerationStructureBuildGeometryInfo = {
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
		.pNext = nullptr,
		.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
		.flags = 0,
		.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
		.srcAccelerationStructure = VK_NULL_HANDLE,
		.dstAccelerationStructure = VK_NULL_HANDLE,
		.geometryCount = 1,
		.pGeometries = &topLevelAccelerationStructureGeometry,
		.ppGeometries = NULL,
		.scratchData = {
			.deviceAddress = 0 }
	};

	VkAccelerationStructureBuildSizesInfoKHR topLevelAccelerationStructureBuildSizesInfo = {
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
		.pNext = nullptr,
		.accelerationStructureSize = 0,
		.updateScratchSize = 0,
		.buildScratchSize = 0
	};

	AlignedVector< uint32_t > topLevelMaxPrimitiveCountList = { numInstances };
	vkGetAccelerationStructureBuildSizesKHR(
		context->getLogicalDevice(),
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&topLevelAccelerationStructureBuildGeometryInfo,
		topLevelMaxPrimitiveCountList.ptr(),
		&topLevelAccelerationStructureBuildSizesInfo);

	// Pooled structures grow a ring; each frame writes a slot, and its instance data, no longer read.
	Ref< AccelerationStructureVk > as = new AccelerationStructureVk(context, true, false, pooled);
	as->m_scratchAlignment = getScratchAlignment(context);
	as->m_instanceCapacity = numInstances;
	as->m_topLevelSize = topLevelAccelerationStructureBuildSizesInfo.accelerationStructureSize;
	as->m_topLevelScratchSize = topLevelAccelerationStructureBuildSizesInfo.buildScratchSize;

	if (!as->insertSlot(0))
		return nullptr;

	return as;
}

Ref< AccelerationStructureVk > AccelerationStructureVk::createBottomLevel(Context* context, const Buffer* vertexBuffer, const IVertexLayout* vertexLayout, const Buffer* indexBuffer, IndexType indexType, const AlignedVector< RaytracingPrimitives >& primitives, bool dynamic, bool pooled)
{
	Ref< AccelerationStructureVk > as = new AccelerationStructureVk(context, false, dynamic, pooled);
	as->m_scratchAlignment = getScratchAlignment(context);

	// First slot is built here, thus read as soon as the structure is referenced.
	if (!as->insertSlot(0))
		return nullptr;
	as->m_slots[0].used = true;

	// Create structure and buffers now so it can be referenced right away; the build is deferred
	// into the upload command buffer, after queued vertex/index uploads, batched with other builds.
	GeometryBuild build;
	if (!as->prepareGeometry(vertexBuffer->getBufferView(), vertexLayout, indexBuffer->getBufferView(), indexType, primitives, true, VK_NULL_HANDLE, build))
		return nullptr;

	// Static structures are never updated, thus scratch is only needed until built.
	Ref< ApiBuffer > scratchBuffer = as->m_scratchBuffers[as->m_index];
	const uint32_t scratchSize = scratchBuffer->getSize();
	if (!dynamic)
		as->m_scratchBuffers[as->m_index] = nullptr;

	// Buffers are referenced until the build has been recorded, and consumed.
	Ref< const Buffer > buildVertexBuffer = vertexBuffer;
	Ref< const Buffer > buildIndexBuffer = indexBuffer;

	context->addDeferredUpload(
		[as, buildVertexBuffer, buildIndexBuffer, build, scratchBuffer, dynamic](Context* cx, CommandBuffer* commandBuffer) mutable {
			// Uploads recorded ahead of the build must be visible to it.
			const VkMemoryBarrier mb = {
				.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
				.pNext = nullptr,
				.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
				.dstAccessMask = VK_ACCESS_SHADER_READ_BIT
			};
			vkCmdPipelineBarrier(
				*commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
				0,
				1,
				&mb,
				0,
				nullptr,
				0,
				nullptr);

			recordGeometry(commandBuffer, build);

			// Released once the upload command buffer has been consumed.
			if (!dynamic)
				safeDestroy(scratchBuffer);
		},
		scratchSize);

	return as;
}

void AccelerationStructureVk::destroy()
{
	// Only relinquish ownership; pending renders may still bind the structure.
}

void AccelerationStructureVk::nextFrame()
{
	if (m_pooled)
		m_pendingFrames++;
}

bool AccelerationStructureVk::writeInstances(CommandBuffer* commandBuffer, const AlignedVector< Instance >& instances)
{
	if (instances.size() > m_instanceCapacity)
	{
		log::error << L"Too many instances in top level acceleration structure (" << (uint32_t)instances.size() << L" > " << m_instanceCapacity << L")." << Endl;
		return false;
	}

	// Readers resolve the current slot when their descriptors are recorded.
	beginWrite();

	ApiBuffer* instanceBuffer = m_instanceBuffers[m_index];
	VkAccelerationStructureInstanceKHR* ptr = (VkAccelerationStructureInstanceKHR*)instanceBuffer->lock();
	if (!ptr)
		return false;

	for (const auto& instance : instances)
	{
		const BufferStaticVk* vd = checked_type_cast< const BufferStaticVk* >(instance.perVertexData);

		const VkAccelerationStructureDeviceAddressInfoKHR asai = {
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
			.pNext = nullptr,
			.accelerationStructure = mandatory_non_null_type_cast< const AccelerationStructureVk* >(instance.blas)->getVkAccelerationStructureKHR()
		};

		const VkDeviceAddress deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(m_context->getLogicalDevice(), &asai);

		const auto& M = instance.transform;
		*ptr++ = {
			.transform = {
				.matrix = {
					{ M(0, 0), M(0, 1), M(0, 2), M(0, 3) },
					{ M(1, 0), M(1, 1), M(1, 2), M(1, 3) },
					{ M(2, 0), M(2, 1), M(2, 2), M(2, 3) } } },
			.instanceCustomIndex = (vd != nullptr) ? vd->getApiBuffer()->makeResourceIndex() : ~0U,
			.mask = 0xff,
			.instanceShaderBindingTableRecordOffset = 0,
			.flags = VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_KHR | VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR,
			.accelerationStructureReference = deviceAddress
		};
	}

	instanceBuffer->unlock();

	const VkAccelerationStructureGeometryDataKHR topLevelAccelerationStructureGeometryData = {
		.instances = {
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
			.pNext = nullptr,
			.arrayOfPointers = VK_FALSE,
			.data = {
				.deviceAddress = instanceBuffer->getDeviceAddress() } }
	};

	const VkAccelerationStructureGeometryKHR topLevelAccelerationStructureGeometry = {
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
		.pNext = nullptr,
		.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
		.geometry = topLevelAccelerationStructureGeometryData,
		.flags = VK_GEOMETRY_OPAQUE_BIT_KHR
	};

	const VkAccelerationStructureBuildGeometryInfoKHR topLevelAccelerationStructureBuildGeometryInfo = {
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
		.pNext = nullptr,
		.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
		.flags = 0,
		.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
		.srcAccelerationStructure = VK_NULL_HANDLE,
		.dstAccelerationStructure = m_as[m_index],
		.geometryCount = 1,
		.pGeometries = &topLevelAccelerationStructureGeometry,
		.ppGeometries = NULL,
		.scratchData = {
			.deviceAddress = alignUp(m_scratchBuffers[m_index]->getDeviceAddress(), m_scratchAlignment) }
	};

	const VkAccelerationStructureBuildRangeInfoKHR topLevelAccelerationStructureBuildRangeInfo = {
		.primitiveCount = (uint32_t)instances.size(),
		.primitiveOffset = 0,
		.firstVertex = 0,
		.transformOffset = 0
	};

	const VkAccelerationStructureBuildRangeInfoKHR* topLevelAccelerationStructureBuildRangeInfos = &topLevelAccelerationStructureBuildRangeInfo;
	vkCmdBuildAccelerationStructuresKHR(
		*commandBuffer,
		1,
		&topLevelAccelerationStructureBuildGeometryInfo,
		&topLevelAccelerationStructureBuildRangeInfos);

	return true;
}

bool AccelerationStructureVk::writeGeometry(CommandBuffer* commandBuffer, const IBufferView* vertexBuffer, const IVertexLayout* vertexLayout, const IBufferView* indexBuffer, IndexType indexType, const AlignedVector< RaytracingPrimitives >& primitives, bool rebuild)
{
	// Refit reads the structure written last.
	const VkAccelerationStructureKHR source = m_as[m_index];
	beginWrite();

	GeometryBuild build;
	if (!prepareGeometry(vertexBuffer, vertexLayout, indexBuffer, indexType, primitives, rebuild, source, build))
		return false;

	recordGeometry(commandBuffer, build);
	return true;
}

AccelerationStructureVk::AccelerationStructureVk(Context* context, bool topLevel, bool dynamic, bool pooled)
	: m_context(context)
	, m_topLevel(topLevel)
	, m_dynamic(dynamic)
	, m_pooled(pooled)
{
}

void AccelerationStructureVk::teardown()
{
	if (m_context != nullptr)
	{
		for (VkAccelerationStructureKHR as : m_as)
		{
			if (as == 0)
				continue;
			m_context->addDeferredCleanup(
				[as](Context* cx) {
				vkDestroyAccelerationStructureKHR(cx->getLogicalDevice(), as, nullptr);
				},
				Context::CleanupFreeDescriptorSets);
		}
	}
	m_as.clear();
	for (auto& instanceBuffer : m_instanceBuffers)
		safeDestroy(instanceBuffer);
	m_instanceBuffers.clear();
	for (auto& hierarchyBuffer : m_hierarchyBuffers)
		safeDestroy(hierarchyBuffer);
	m_hierarchyBuffers.clear();
	for (auto& scratchBuffer : m_scratchBuffers)
		safeDestroy(scratchBuffer);
	m_scratchBuffers.clear();
	m_slots.clear();
	m_context = nullptr;
}

void AccelerationStructureVk::beginWrite()
{
	// Writes are rendered on a single thread, thus only nextFrame changes the count meanwhile.
	if (m_pendingFrames > 0)
	{
		m_pendingFrames--;
		advance();
	}
}

void AccelerationStructureVk::advance()
{
	const uint64_t submittedFrames = m_context->getSubmittedFrameCount();

	// Readers of the slot being left are submitted with the frame being recorded.
	Slot& current = m_slots[m_index];
	if (current.used)
		current.leftFrame = submittedFrames;

	uint32_t next = (m_index + 1) % (uint32_t)m_as.size();
	if (!isSlotFree(next, submittedFrames))
	{
		if (insertSlot(m_index + 1))
		{
			next = m_index + 1;
			log::debug << (m_topLevel ? L"Top" : L"Bottom") << L" level acceleration structure ring grown to " << (uint32_t)m_as.size() << L" slots." << Endl;
		}
		else
			log::error << L"Unable to grow acceleration structure ring; slot rewritten while in use." << Endl;
	}

	m_index = next;
	m_slots[m_index].used = true;
}

bool AccelerationStructureVk::isSlotFree(uint32_t slot, uint64_t submittedFrames) const
{
	const Slot& s = m_slots[slot];
	if (!s.used)
		return true;

	// Left by the frame being recorded; its readers might not be submitted yet.
	if (submittedFrames <= s.leftFrame)
		return false;

	// The first frame submitted after the slot was left carries all of its readers.
	return m_context->getCompletedEpoch() >= m_context->getSubmittedFrameEpoch(s.leftFrame + 1);
}

bool AccelerationStructureVk::insertSlot(uint32_t at)
{
	m_instanceBuffers.insert(m_instanceBuffers.begin() + at, nullptr);
	m_hierarchyBuffers.insert(m_hierarchyBuffers.begin() + at, nullptr);
	m_scratchBuffers.insert(m_scratchBuffers.begin() + at, nullptr);
	m_as.insert(m_as.begin() + at, VK_NULL_HANDLE);
	m_slots.insert(m_slots.begin() + at, Slot());

	// Keep the current slot current; inserting at or before it shifts it.
	if (m_as.size() > 1 && at <= m_index)
		m_index++;

	// Bottom level slots get their buffers and structure when first built.
	if (m_topLevel && !createTopLevelSlot(at))
	{
		m_instanceBuffers.erase(m_instanceBuffers.begin() + at);
		m_hierarchyBuffers.erase(m_hierarchyBuffers.begin() + at);
		m_scratchBuffers.erase(m_scratchBuffers.begin() + at);
		m_as.erase(m_as.begin() + at);
		m_slots.erase(m_slots.begin() + at);
		if (m_as.size() > 0 && at < m_index)
			m_index--;
		return false;
	}

	return true;
}

bool AccelerationStructureVk::createTopLevelSlot(uint32_t slot)
{
	// Instance data; written by the CPU, read by the build of this slot.
	Ref< ApiBuffer > instanceBuffer = new ApiBuffer(m_context);
	if (!instanceBuffer->create(
			m_instanceCapacity * sizeof(VkAccelerationStructureInstanceKHR),
			VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			true,
			true))
		return false;

	// Hierarchy; shared concurrently since built on compute and read by graphics ray queries.
	Ref< ApiBuffer > hierarchyBuffer = new ApiBuffer(m_context);
	if (!hierarchyBuffer->create(
			(uint32_t)m_topLevelSize,
			VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
			false,
			true,
			true))
	{
		safeDestroy(instanceBuffer);
		return false;
	}

	// Create scratch buffer used when building the hierarchy.
	Ref< ApiBuffer > scratchBuffer = new ApiBuffer(m_context);
	if (!scratchBuffer->create(
			(uint32_t)m_topLevelScratchSize + m_scratchAlignment,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			false,
			true))
	{
		safeDestroy(instanceBuffer);
		safeDestroy(hierarchyBuffer);
		return false;
	}

	// Create AS object.
	const VkAccelerationStructureCreateInfoKHR topLevelAccelerationStructureCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
		.pNext = nullptr,
		.createFlags = 0,
		.buffer = *hierarchyBuffer,
		.offset = 0,
		.size = m_topLevelSize,
		.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
		.deviceAddress = 0
	};

	VkAccelerationStructureKHR accelerationStructure = VK_NULL_HANDLE;
	if (vkCreateAccelerationStructureKHR(
			m_context->getLogicalDevice(),
			&topLevelAccelerationStructureCreateInfo,
			nullptr,
			&accelerationStructure) != VK_SUCCESS)
	{
		safeDestroy(instanceBuffer);
		safeDestroy(hierarchyBuffer);
		safeDestroy(scratchBuffer);
		return false;
	}

	m_instanceBuffers[slot] = instanceBuffer;
	m_hierarchyBuffers[slot] = hierarchyBuffer;
	m_scratchBuffers[slot] = scratchBuffer;
	m_as[slot] = accelerationStructure;
	return true;
}

bool AccelerationStructureVk::prepareGeometry(const IBufferView* vertexBuffer, const IVertexLayout* vertexLayout, const IBufferView* indexBuffer, IndexType indexType, const AlignedVector< RaytracingPrimitives >& primitives, bool rebuild, VkAccelerationStructureKHR source, GeometryBuild& outBuild)
{
	bool recreateAS = false;
	VkResult result;

	const VertexLayoutVk* vertexLayoutVk = mandatory_non_null_type_cast< const VertexLayoutVk* >(vertexLayout);
	const int32_t pidx = vertexLayoutVk->getPositionElementIndex();
	if (pidx < 0)
		return false;

	const VkVertexInputAttributeDescription& piad = vertexLayoutVk->getVkVertexInputAttributeDescriptions()[pidx];

	const BufferViewVk* vb = mandatory_non_null_type_cast< const BufferViewVk* >(vertexBuffer);
	const BufferViewVk* ib = mandatory_non_null_type_cast< const BufferViewVk* >(indexBuffer);

	const VkAccelerationStructureGeometryDataKHR bottomLevelAccelerationStructureGeometryData = {
		.triangles = {
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
			.pNext = nullptr,
			.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT,
			.vertexData = {
				.deviceAddress = vb->getDeviceAddress(m_context) },
			.vertexStride = vertexLayoutVk->getVkVertexInputBindingDescription().stride,
			.maxVertex = vb->getVkBufferSize() / vertexLayoutVk->getVkVertexInputBindingDescription().stride,
			.indexType = (indexType == IndexType::UInt32) ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16,
			.indexData = { .deviceAddress = ib->getDeviceAddress(m_context) },
			.transformData = { .deviceAddress = 0 } }
	};

	// One geometry per primitive range, all sharing vertex and index data; the structure is
	// sized for these ranges only, as the index buffer may hold more (other LODs, parts).
	AlignedVector< VkAccelerationStructureBuildRangeInfoKHR >& buildRanges = outBuild.ranges;
	buildRanges.resize(0);
	for (const auto& rtp : primitives)
	{
		const auto& primitives = rtp.primitives;

		if (
			primitives.type != PrimitiveType::Triangles ||
			primitives.indexed == false)
			continue;

		buildRanges.push_back({ .primitiveCount = primitives.count,
			.primitiveOffset = primitives.offset * ((indexType == IndexType::UInt32) ? 4 : 2),
			.firstVertex = rtp.firstVertex,
			.transformOffset = 0 });
	}
	if (buildRanges.empty())
		return false;

	const VkAccelerationStructureGeometryKHR bottomLevelAccelerationStructureGeometry = {
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
		.pNext = nullptr,
		.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
		.geometry = bottomLevelAccelerationStructureGeometryData,
		.flags = VK_GEOMETRY_OPAQUE_BIT_KHR
	};

	AlignedVector< VkAccelerationStructureGeometryKHR >& bottomLevelAccelerationStructureGeometries = outBuild.geometries;
	bottomLevelAccelerationStructureGeometries.resize(0);
	bottomLevelAccelerationStructureGeometries.resize(buildRanges.size(), bottomLevelAccelerationStructureGeometry);
	AlignedVector< uint32_t > bottomLevelMaxPrimitiveCountList;
	for (const auto& buildRange : buildRanges)
		bottomLevelMaxPrimitiveCountList.push_back(buildRange.primitiveCount);

	VkAccelerationStructureBuildGeometryInfoKHR bottomLevelAccelerationStructureBuildGeometryInfo = {
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
		.pNext = nullptr,
		.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
		.flags = (VkBuildAccelerationStructureFlagsKHR)(m_dynamic ? VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR : 0),
		.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
		.srcAccelerationStructure = VK_NULL_HANDLE,
		.dstAccelerationStructure = VK_NULL_HANDLE,
		.geometryCount = (uint32_t)bottomLevelAccelerationStructureGeometries.size(),
		.pGeometries = bottomLevelAccelerationStructureGeometries.ptr(),
		.ppGeometries = nullptr,
		.scratchData = {
			.deviceAddress = 0 }
	};

	VkAccelerationStructureBuildSizesInfoKHR bottomLevelAccelerationStructureBuildSizesInfo = {
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
		.pNext = nullptr,
		.accelerationStructureSize = 0,
		.updateScratchSize = 0,
		.buildScratchSize = 0
	};

	vkGetAccelerationStructureBuildSizesKHR(
		m_context->getLogicalDevice(),
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&bottomLevelAccelerationStructureBuildGeometryInfo,
		bottomLevelMaxPrimitiveCountList.ptr(),
		&bottomLevelAccelerationStructureBuildSizesInfo);

	const uint32_t slot = m_index;

	// Re-create buffer to hold AS hierarchical data.
	if (m_hierarchyBuffers[slot] && m_hierarchyBuffers[slot]->getSize() < bottomLevelAccelerationStructureBuildSizesInfo.accelerationStructureSize)
		safeDestroy(m_hierarchyBuffers[slot]);
	if (!m_hierarchyBuffers[slot])
	{
		m_hierarchyBuffers[slot] = new ApiBuffer(m_context);
		if (!m_hierarchyBuffers[slot]->create(
				bottomLevelAccelerationStructureBuildSizesInfo.accelerationStructureSize,
				VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
				false,
				true,
				true))
			return false;

		recreateAS = true;
	}

	// Re-create scratch buffer used when building the hierarchy. Each ring slot has its
	// own scratch so concurrent in-flight builds do not collide on it.
	if (m_scratchBuffers[slot] && m_scratchBuffers[slot]->getSize() < bottomLevelAccelerationStructureBuildSizesInfo.buildScratchSize + m_scratchAlignment)
		safeDestroy(m_scratchBuffers[slot]);
	if (!m_scratchBuffers[slot])
	{
		m_scratchBuffers[slot] = new ApiBuffer(m_context);
		if (!m_scratchBuffers[slot]->create(
				bottomLevelAccelerationStructureBuildSizesInfo.buildScratchSize + m_scratchAlignment,
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
				false,
				true))
		{
			safeDestroy(m_hierarchyBuffers[slot]);
			return false;
		}

		recreateAS = true;
	}

	// Create AS object.
	const VkAccelerationStructureCreateInfoKHR bottomLevelAccelerationStructureCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
		.pNext = nullptr,
		.createFlags = 0,
		.buffer = *m_hierarchyBuffers[slot],
		.offset = 0,
		.size = bottomLevelAccelerationStructureBuildSizesInfo.accelerationStructureSize,
		.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
		.deviceAddress = 0
	};

	if (recreateAS && m_as[slot] != 0)
	{
		if (source == m_as[slot])
			source = VK_NULL_HANDLE;
		m_context->addDeferredCleanup(
			[as = m_as[slot]](Context* cx) {
			vkDestroyAccelerationStructureKHR(cx->getLogicalDevice(), as, nullptr);
			},
			Context::CleanupFreeDescriptorSets);
		m_as[slot] = 0;
	}

	// Refit from the structure written last, rather than rebuild, when dynamic and still valid;
	// requires the ALLOW_UPDATE build flag and unchanged topology.
	if (m_dynamic && !rebuild && source != VK_NULL_HANDLE)
	{
		bottomLevelAccelerationStructureBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
		bottomLevelAccelerationStructureBuildGeometryInfo.srcAccelerationStructure = source;
	}

	// Re-create if necessary.
	if (!m_as[slot])
	{
		result = vkCreateAccelerationStructureKHR(
			m_context->getLogicalDevice(),
			&bottomLevelAccelerationStructureCreateInfo,
			nullptr,
			&m_as[slot]);
		if (result != VK_SUCCESS)
			return false;
	}

	// Build AS.
	bottomLevelAccelerationStructureBuildGeometryInfo.dstAccelerationStructure = m_as[slot];
	bottomLevelAccelerationStructureBuildGeometryInfo.scratchData.deviceAddress = alignUp(m_scratchBuffers[slot]->getDeviceAddress(), m_scratchAlignment);
	outBuild.info = bottomLevelAccelerationStructureBuildGeometryInfo;
	return true;
}

void AccelerationStructureVk::recordGeometry(CommandBuffer* commandBuffer, const GeometryBuild& build)
{
	VkAccelerationStructureBuildGeometryInfoKHR info = build.info;
	info.geometryCount = (uint32_t)build.geometries.size();
	info.pGeometries = build.geometries.c_ptr();

	// A single build info takes one pointer to an array of ranges, one per geometry.
	const VkAccelerationStructureBuildRangeInfoKHR* buildRangePtr = build.ranges.c_ptr();
	vkCmdBuildAccelerationStructuresKHR(
		*commandBuffer,
		1,
		&info,
		&buildRangePtr);
}

}
