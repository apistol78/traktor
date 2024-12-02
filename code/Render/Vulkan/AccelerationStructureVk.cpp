/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Render/Buffer.h"
#include "Render/Vulkan/AccelerationStructureVk.h"
#include "Render/Vulkan/BufferDynamicVk.h"
#include "Render/Vulkan/BufferStaticVk.h"
#include "Render/Vulkan/BufferViewVk.h"
#include "Render/Vulkan/VertexLayoutVk.h"
#include "Render/Vulkan/Private/ApiBuffer.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/Queue.h"

namespace traktor::render
{
	namespace
	{

uint32_t getScratchAlignment(Context* context)
{
	// Determine alignment requirement of scratch buffer.
	VkPhysicalDeviceAccelerationStructurePropertiesKHR asp =
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR
	};
	VkPhysicalDeviceProperties2 deviceProperties =
	{
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
	destroy();
}

Ref< AccelerationStructureVk > AccelerationStructureVk::createTopLevel(Context* context, uint32_t numInstances, uint32_t inFlightCount)
{
	const uint32_t scratchAlignment = getScratchAlignment(context);
	VkResult result;

	// Allocate buffer containing all the transforms and references to BLAS.
	static uint32_t instances = 0;
	Ref< BufferDynamicVk > instanceBuffer = new BufferDynamicVk(
		context,
		numInstances * sizeof(VkAccelerationStructureInstanceKHR),
		instances
	);
	instanceBuffer->create(
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		inFlightCount
	);

	// Create the AS object.
	VkAccelerationStructureGeometryDataKHR topLevelAccelerationStructureGeometryData =
	{
		.instances =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
			.pNext = nullptr,
			.arrayOfPointers = VK_FALSE,
			.data =
			{
				.deviceAddress = static_cast< const BufferViewVk* >(instanceBuffer->getBufferView())->getDeviceAddress(context)
			}
		}
	};

	VkAccelerationStructureGeometryKHR topLevelAccelerationStructureGeometry =
	{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
		.pNext = nullptr,
		.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
		.geometry = topLevelAccelerationStructureGeometryData,
		.flags = VK_GEOMETRY_OPAQUE_BIT_KHR
	};

	VkAccelerationStructureBuildGeometryInfoKHR topLevelAccelerationStructureBuildGeometryInfo =
	{
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
		.scratchData =
		{
			.deviceAddress = 0
		}
	};

	VkAccelerationStructureBuildSizesInfoKHR topLevelAccelerationStructureBuildSizesInfo =
	{
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
		&topLevelAccelerationStructureBuildSizesInfo
	);

	// Create buffer to hold AS hierarchical data.
	Ref< ApiBuffer > hierarchyBuffer = new ApiBuffer(context);
	if (!hierarchyBuffer->create(
		topLevelAccelerationStructureBuildSizesInfo.accelerationStructureSize,
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
		false,
		true
	))
	{
		safeDestroy(instanceBuffer);
		return nullptr;
	}

	// Create scratch buffer used when building the hierarchy.
	Ref< ApiBuffer > scratchBuffer = new ApiBuffer(context);
	if (!scratchBuffer->create(
		topLevelAccelerationStructureBuildSizesInfo.buildScratchSize + scratchAlignment,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		false,
		true
	))
	{
		safeDestroy(instanceBuffer);
		safeDestroy(hierarchyBuffer);
		return nullptr;
	}

	// Create AS object.
	VkAccelerationStructureCreateInfoKHR topLevelAccelerationStructureCreateInfo =
	{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
		.pNext = nullptr,
		.createFlags = 0,
		.buffer = *hierarchyBuffer,
		.offset = 0,
		.size = topLevelAccelerationStructureBuildSizesInfo.accelerationStructureSize,
		.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
		.deviceAddress = 0
	};

	VkAccelerationStructureKHR accelerationStructure = VK_NULL_HANDLE;
	result = vkCreateAccelerationStructureKHR(
		context->getLogicalDevice(),
		&topLevelAccelerationStructureCreateInfo,
		nullptr,
		&accelerationStructure
	);
	if (result != VK_SUCCESS)
	{
		safeDestroy(instanceBuffer);
		safeDestroy(hierarchyBuffer);
		safeDestroy(scratchBuffer);
		return nullptr;
	}

	Ref< AccelerationStructureVk > as = new AccelerationStructureVk(context);
	as->m_hierarchyBuffer = hierarchyBuffer;
	as->m_instanceBuffer = instanceBuffer;
	as->m_scratchBuffer = scratchBuffer;
	as->m_as = accelerationStructure;
	as->m_scratchAlignment = scratchAlignment;
	return as;
}

Ref< AccelerationStructureVk > AccelerationStructureVk::createBottomLevel(Context* context, const Buffer* vertexBuffer, const IVertexLayout* vertexLayout, const Buffer* indexBuffer, IndexType indexType, const AlignedVector< Primitives >& primitives)
{
	auto commandBuffer = context->getGraphicsQueue()->acquireCommandBuffer(L"AccelerationStructureVk::createBottomLevel");

	Ref< AccelerationStructureVk > as = new AccelerationStructureVk(context);
	as->m_scratchAlignment = getScratchAlignment(context);
	as->writeGeometry(commandBuffer, vertexBuffer->getBufferView(), vertexLayout, indexBuffer->getBufferView(), indexType, primitives);

	commandBuffer->submit({}, {}, VK_NULL_HANDLE);

	context->addDeferredCleanup(
		[=](Context* cx) { commandBuffer->wait(); },
		Context::CleanupNone
	);

	return as;
}

void AccelerationStructureVk::destroy()
{
	if (m_context != nullptr)
	{
		m_context->addDeferredCleanup(
			[as = m_as](Context* cx) { vkDestroyAccelerationStructureKHR(cx->getLogicalDevice(), as, nullptr); },
			Context::CleanupNeedFlushGPU | Context::CleanupFreeDescriptorSets
		);
		m_as = 0;
	}
	safeDestroy(m_instanceBuffer);
	safeDestroy(m_hierarchyBuffer);
	safeDestroy(m_scratchBuffer);
	m_context = nullptr;
}

bool AccelerationStructureVk::writeInstances(CommandBuffer* commandBuffer, const AlignedVector< Instance >& instances)
{
	VkAccelerationStructureInstanceKHR* ptr = (VkAccelerationStructureInstanceKHR*)m_instanceBuffer->lock();
	if (!ptr)
		return false;

	for (const auto& instance : instances)
	{
		const BufferStaticVk* vd = checked_type_cast< const BufferStaticVk* >( instance.perVertexData );

		const VkAccelerationStructureDeviceAddressInfoKHR asai =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
			.pNext = nullptr,
			.accelerationStructure = mandatory_non_null_type_cast< const AccelerationStructureVk* >(instance.blas)->getVkAccelerationStructureKHR()
		};

		const VkDeviceAddress deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(m_context->getLogicalDevice(), &asai);

		const auto& M = instance.transform;
		*ptr++ =
		{
			.transform =
			{
				.matrix =
				{
					{ M(0, 0), M(0, 1), M(0, 2), M(0, 3) },
					{ M(1, 0), M(1, 1), M(1, 2), M(1, 3) },
					{ M(2, 0), M(2, 1), M(2, 2), M(2, 3) }
				}
			},
			.instanceCustomIndex = (vd != nullptr) ? vd->getApiBuffer()->makeResourceIndex() : ~0U,
			.mask = 0xff,
			.instanceShaderBindingTableRecordOffset = 0,
			.flags = VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_KHR | VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR,
			.accelerationStructureReference = deviceAddress
		};
	}

	m_instanceBuffer->unlock();

	const VkAccelerationStructureGeometryDataKHR topLevelAccelerationStructureGeometryData =
	{
		.instances =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
			.pNext = nullptr,
			.arrayOfPointers = VK_FALSE,
			.data =
			{
				.deviceAddress = static_cast< const BufferViewVk* >(m_instanceBuffer->getBufferView())->getDeviceAddress(m_context)
			}
		}
	};

	const VkAccelerationStructureGeometryKHR topLevelAccelerationStructureGeometry =
	{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
		.pNext = nullptr,
		.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
		.geometry = topLevelAccelerationStructureGeometryData,
		.flags = VK_GEOMETRY_OPAQUE_BIT_KHR
	};

	const VkAccelerationStructureBuildGeometryInfoKHR topLevelAccelerationStructureBuildGeometryInfo =
	{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
		.pNext = nullptr,
		.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
		.flags = 0,
		.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
		.srcAccelerationStructure = VK_NULL_HANDLE,
		.dstAccelerationStructure = m_as,
		.geometryCount = 1,
		.pGeometries = &topLevelAccelerationStructureGeometry,
		.ppGeometries = NULL,
		.scratchData =
		{
			.deviceAddress = alignUp(m_scratchBuffer->getDeviceAddress(), m_scratchAlignment)
		}
	};

	const VkAccelerationStructureBuildRangeInfoKHR topLevelAccelerationStructureBuildRangeInfo =
	{
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
		&topLevelAccelerationStructureBuildRangeInfos
	);

	return true;
}

bool AccelerationStructureVk::writeGeometry(CommandBuffer* commandBuffer, const IBufferView* vertexBuffer, const IVertexLayout* vertexLayout, const IBufferView* indexBuffer, IndexType indexType, const AlignedVector< Primitives >& primitives)
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

	const VkAccelerationStructureGeometryDataKHR bottomLevelAccelerationStructureGeometryData =
	{
		.triangles =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
			.pNext = nullptr,
			.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT,
			.vertexData =
			{
				.deviceAddress = vb->getDeviceAddress(m_context)
			},
			.vertexStride = vertexLayoutVk->getVkVertexInputBindingDescription().stride,
			.maxVertex = vb->getVkBufferSize() / vertexLayoutVk->getVkVertexInputBindingDescription().stride,
			.indexType = (indexType == IndexType::UInt32) ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16,
			.indexData =
			{
				.deviceAddress = ib->getDeviceAddress(m_context)
			},
			.transformData =
			{
				.deviceAddress = 0
			}
		}
	};

	const VkAccelerationStructureGeometryKHR bottomLevelAccelerationStructureGeometry =
	{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
		.pNext = nullptr,
		.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
		.geometry = bottomLevelAccelerationStructureGeometryData,
		.flags = VK_GEOMETRY_OPAQUE_BIT_KHR
	};

	VkAccelerationStructureBuildGeometryInfoKHR bottomLevelAccelerationStructureBuildGeometryInfo =
	{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
		.pNext = nullptr,
		.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
		.flags = 0,
		.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
		.srcAccelerationStructure = VK_NULL_HANDLE,
		.dstAccelerationStructure = VK_NULL_HANDLE,
		.geometryCount = 1,
		.pGeometries = &bottomLevelAccelerationStructureGeometry,
		.ppGeometries = nullptr,
		.scratchData =
		{
			.deviceAddress = 0
		}
	};

	VkAccelerationStructureBuildSizesInfoKHR bottomLevelAccelerationStructureBuildSizesInfo =
	{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
		.pNext = nullptr,
		.accelerationStructureSize = 0,
		.updateScratchSize = 0,
		.buildScratchSize = 0
	};

	const uint32_t primitiveCount = ib->getVkBufferSize() / ((indexType == IndexType::UInt32) ? 4 : 2);
	AlignedVector< uint32_t > bottomLevelMaxPrimitiveCountList = { primitiveCount };
	vkGetAccelerationStructureBuildSizesKHR(
		m_context->getLogicalDevice(),
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&bottomLevelAccelerationStructureBuildGeometryInfo,
		bottomLevelMaxPrimitiveCountList.ptr(),
		&bottomLevelAccelerationStructureBuildSizesInfo
	);

	// Re-create buffer to hold AS hierarchical data.
	if (m_hierarchyBuffer && m_hierarchyBuffer->getSize() < bottomLevelAccelerationStructureBuildSizesInfo.accelerationStructureSize)
		safeDestroy(m_hierarchyBuffer);
	if (!m_hierarchyBuffer)
	{
		m_hierarchyBuffer = new ApiBuffer(m_context);
		if (!m_hierarchyBuffer->create(
			bottomLevelAccelerationStructureBuildSizesInfo.accelerationStructureSize,
			VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			false,
			true
		))
			return false;

		recreateAS = true;
	}

	// Re-create scratch buffer used when building the hierarchy.
	if (m_scratchBuffer && m_scratchBuffer->getSize() < bottomLevelAccelerationStructureBuildSizesInfo.buildScratchSize + m_scratchAlignment)
		safeDestroy(m_scratchBuffer);
	if (!m_scratchBuffer)
	{
		m_scratchBuffer = new ApiBuffer(m_context);
		if (!m_scratchBuffer->create(
			bottomLevelAccelerationStructureBuildSizesInfo.buildScratchSize + m_scratchAlignment,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			false,
			true
		))
		{
			safeDestroy(m_hierarchyBuffer);
			return false;
		}

		recreateAS = true;
	}

	// Create AS object.
	const VkAccelerationStructureCreateInfoKHR bottomLevelAccelerationStructureCreateInfo =
	{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
		.pNext = nullptr,
		.createFlags = 0,
		.buffer = *m_hierarchyBuffer,
		.offset = 0,
		.size = bottomLevelAccelerationStructureBuildSizesInfo.accelerationStructureSize,
		.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
		.deviceAddress = 0
	};

	if (recreateAS && m_as != 0)
	{
		m_context->addDeferredCleanup(
			[as = m_as](Context* cx) { vkDestroyAccelerationStructureKHR(cx->getLogicalDevice(), as, nullptr); },
			Context::CleanupNeedFlushGPU | Context::CleanupFreeDescriptorSets
		);
		m_as = 0;
	}

	// Re-create if necessary.
	if (!m_as)
	{
		result = vkCreateAccelerationStructureKHR(
			m_context->getLogicalDevice(),
			&bottomLevelAccelerationStructureCreateInfo,
			nullptr,
			&m_as
		);
		if (result != VK_SUCCESS)
			return false;
	}

	// Build AS.
	bottomLevelAccelerationStructureBuildGeometryInfo.dstAccelerationStructure = m_as;
	bottomLevelAccelerationStructureBuildGeometryInfo.scratchData.deviceAddress = alignUp(m_scratchBuffer->getDeviceAddress(), m_scratchAlignment);

	AlignedVector< VkAccelerationStructureBuildRangeInfoKHR > buildRanges;
	for (const auto& primitive : primitives)
	{
		if (
			primitive.type != PrimitiveType::Triangles ||
			primitive.indexed == false
		)
			continue;

		VkAccelerationStructureBuildRangeInfoKHR& offset = buildRanges.push_back();
		offset.firstVertex = 0;
		offset.primitiveCount = primitive.count;
		offset.primitiveOffset = primitive.offset * ((indexType == IndexType::UInt32) ? 4 : 2);
		offset.transformOffset = 0;
	}

	AlignedVector< VkAccelerationStructureBuildRangeInfoKHR* > buildRangePtrs;
	for (auto& buildRange : buildRanges)
		buildRangePtrs.push_back(&buildRange);

	vkCmdBuildAccelerationStructuresKHR(
		*commandBuffer,
		1,
		&bottomLevelAccelerationStructureBuildGeometryInfo,
		buildRangePtrs.ptr()
	);

	return true;
}

AccelerationStructureVk::AccelerationStructureVk(Context* context)
:	m_context(context)
{
}

}
