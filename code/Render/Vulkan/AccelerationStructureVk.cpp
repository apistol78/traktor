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
#include "Render/Vulkan/BufferViewVk.h"
#include "Render/Vulkan/VertexLayoutVk.h"
#include "Render/Vulkan/Private/ApiBuffer.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/Queue.h"

namespace traktor::render
{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.AccelerationStructureVk", AccelerationStructureVk, IAccelerationStructure)

AccelerationStructureVk::~AccelerationStructureVk()
{
	if (m_context != nullptr)
	{
		m_context->addDeferredCleanup([
			as = m_as
		](Context* cx) {
			vkDestroyAccelerationStructureKHR(cx->getLogicalDevice(), as, nullptr);
		});
	}
	safeDestroy(m_hierarchyBuffer);
	safeDestroy(m_instanceBuffer);
	safeDestroy(m_scratchBuffer);
	m_context = nullptr;
}

Ref< AccelerationStructureVk > AccelerationStructureVk::createTopLevel(Context* context, uint32_t numInstances)
{
	VkResult result;

	// Allocate buffer containing all the transforms and references to BLAS.
	Ref< ApiBuffer > instanceBuffer = new ApiBuffer(context);
	instanceBuffer->create(
		numInstances * sizeof(VkAccelerationStructureInstanceKHR),
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		true,
		true
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
				.deviceAddress = instanceBuffer->getDeviceAddress()
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

	// Create buffer to hold AS hierarchial data.
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
		topLevelAccelerationStructureBuildSizesInfo.buildScratchSize,
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
		return nullptr;

	return new AccelerationStructureVk(context, hierarchyBuffer, instanceBuffer, scratchBuffer, accelerationStructure);
}

Ref< AccelerationStructureVk > AccelerationStructureVk::createBottomLevel(Context* context, const Buffer* vertexBuffer, const IVertexLayout* vertexLayout, const Buffer* indexBuffer, IndexType indexType, const AlignedVector< Primitives >& primitives)
{
	VkResult result;

	const VertexLayoutVk* vertexLayoutVk = mandatory_non_null_type_cast< const VertexLayoutVk* >(vertexLayout);
	const int32_t pidx = vertexLayoutVk->getPositionElementIndex();
	if (pidx < 0)
		return nullptr;

	const VkVertexInputAttributeDescription& piad = vertexLayoutVk->getVkVertexInputAttributeDescriptions()[pidx];

	const BufferViewVk* vb = mandatory_non_null_type_cast< const BufferViewVk* >(vertexBuffer->getBufferView());
	const BufferViewVk* ib = mandatory_non_null_type_cast< const BufferViewVk* >(indexBuffer->getBufferView());

	VkAccelerationStructureGeometryDataKHR bottomLevelAccelerationStructureGeometryData =
	{
		.triangles =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
			.pNext = nullptr,
			.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT,
			.vertexData =
			{
				.deviceAddress = vb->getDeviceAddress(context)
			},
			.vertexStride = vertexLayoutVk->getVkVertexInputBindingDescription().stride,
			.maxVertex = vb->getVkBufferSize() / vertexLayoutVk->getVkVertexInputBindingDescription().stride,
			.indexType = (indexType == IndexType::UInt32) ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16,
			.indexData =
			{
				.deviceAddress = ib->getDeviceAddress(context)
			},
			.transformData =
			{
				.deviceAddress = 0
			}
		}
	};

	VkAccelerationStructureGeometryKHR bottomLevelAccelerationStructureGeometry =
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

	const uint32_t primitiveCount = indexBuffer->getBufferSize() / ((indexType == IndexType::UInt32) ? 4 : 2);
	AlignedVector< uint32_t > bottomLevelMaxPrimitiveCountList = { primitiveCount };
	vkGetAccelerationStructureBuildSizesKHR(
		context->getLogicalDevice(),
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&bottomLevelAccelerationStructureBuildGeometryInfo,
		bottomLevelMaxPrimitiveCountList.ptr(),
		&bottomLevelAccelerationStructureBuildSizesInfo
	);

	// Create buffer to hold AS hierarchial data.
	Ref< ApiBuffer > hierarchyBuffer = new ApiBuffer(context);
	if (!hierarchyBuffer->create(
		bottomLevelAccelerationStructureBuildSizesInfo.accelerationStructureSize,
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		false,
		true
	))
		return nullptr;

	// Create scratch buffer used when building the hierarchy.
	Ref< ApiBuffer > scratchBuffer = new ApiBuffer(context);
	if (!scratchBuffer->create(
		bottomLevelAccelerationStructureBuildSizesInfo.buildScratchSize,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		false,
		true
	))
	{
		safeDestroy(hierarchyBuffer);
		return nullptr;
	}

	// Create AS object.
	VkAccelerationStructureCreateInfoKHR bottomLevelAccelerationStructureCreateInfo =
	{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
		.pNext = nullptr,
		.createFlags = 0,
		.buffer = *hierarchyBuffer,
		.offset = 0,
		.size = bottomLevelAccelerationStructureBuildSizesInfo.accelerationStructureSize,
		.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
		.deviceAddress = 0
	};

	VkAccelerationStructureKHR accelerationStructure = VK_NULL_HANDLE;
	result = vkCreateAccelerationStructureKHR(
		context->getLogicalDevice(),
		&bottomLevelAccelerationStructureCreateInfo,
		nullptr,
		&accelerationStructure
	);
	if (result != VK_SUCCESS)
		return nullptr;

	// Build AS.
	bottomLevelAccelerationStructureBuildGeometryInfo.dstAccelerationStructure = accelerationStructure;
	bottomLevelAccelerationStructureBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer->getDeviceAddress();

	AlignedVector< VkAccelerationStructureBuildRangeInfoKHR > buildRanges;
	for (const auto& primitive : primitives)
	{
		if (primitive.type != PrimitiveType::Triangles)
			continue;

		VkAccelerationStructureBuildRangeInfoKHR& offset = buildRanges.push_back();
		offset.firstVertex = primitive.offset;
		offset.primitiveCount = primitive.count;
		offset.primitiveOffset = 0;
		offset.transformOffset = 0;
	}

	AlignedVector< VkAccelerationStructureBuildRangeInfoKHR* > buildRangePtrs;
	for (auto& buildRange : buildRanges)
		buildRangePtrs.push_back(&buildRange);

	auto commandBuffer = context->getGraphicsQueue()->acquireCommandBuffer(T_FILE_LINE_W);
	vkCmdBuildAccelerationStructuresKHR(
		*commandBuffer,
		1,
		&bottomLevelAccelerationStructureBuildGeometryInfo,
		buildRangePtrs.ptr()
	);
	commandBuffer->submitAndWait();

	return new AccelerationStructureVk(context, hierarchyBuffer, scratchBuffer, accelerationStructure);
}

bool AccelerationStructureVk::writeInstances(const AlignedVector< Instance >& instances)
{
	VkAccelerationStructureInstanceKHR* ptr = (VkAccelerationStructureInstanceKHR*)m_instanceBuffer->lock();
	if (!ptr)
		return false;

	for (const auto& instance : instances)
	{
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
			.instanceCustomIndex = 0,
			.mask = 0xff,
			.instanceShaderBindingTableRecordOffset = 0,
			.flags = VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_KHR | VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR,
			.accelerationStructureReference = deviceAddress
		};
	}

	m_instanceBuffer->unlock();

	VkAccelerationStructureGeometryDataKHR topLevelAccelerationStructureGeometryData =
	{
		.instances =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
			.pNext = nullptr,
			.arrayOfPointers = VK_FALSE,
			.data =
			{
				.deviceAddress = m_instanceBuffer->getDeviceAddress()
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
		.dstAccelerationStructure = m_as,
		.geometryCount = 1,
		.pGeometries = &topLevelAccelerationStructureGeometry,
		.ppGeometries = NULL,
		.scratchData =
		{
			.deviceAddress = m_scratchBuffer->getDeviceAddress()
		}
	};

	VkAccelerationStructureBuildRangeInfoKHR topLevelAccelerationStructureBuildRangeInfo =
	{
		.primitiveCount = (uint32_t)instances.size(),
		.primitiveOffset = 0,
		.firstVertex = 0,
		.transformOffset = 0
	};

	const VkAccelerationStructureBuildRangeInfoKHR* topLevelAccelerationStructureBuildRangeInfos = &topLevelAccelerationStructureBuildRangeInfo;

	auto commandBuffer = m_context->getGraphicsQueue()->acquireCommandBuffer(T_FILE_LINE_W);
	vkCmdBuildAccelerationStructuresKHR(
		*commandBuffer,
		1,
		&topLevelAccelerationStructureBuildGeometryInfo,
		&topLevelAccelerationStructureBuildRangeInfos
	);
	commandBuffer->submitAndWait();

	return true;
}

AccelerationStructureVk::AccelerationStructureVk(Context* context, ApiBuffer* hierarchyBuffer, ApiBuffer* instanceBuffer, ApiBuffer* scratchBuffer, VkAccelerationStructureKHR as)
:	m_context(context)
,	m_hierarchyBuffer(hierarchyBuffer)
,	m_instanceBuffer(instanceBuffer)
,	m_scratchBuffer(scratchBuffer)
,	m_as(as)
{
}

AccelerationStructureVk::AccelerationStructureVk(Context* context, ApiBuffer* hierarchyBuffer, ApiBuffer* scratchBuffer, VkAccelerationStructureKHR as)
:	m_context(context)
,	m_hierarchyBuffer(hierarchyBuffer)
,	m_scratchBuffer(scratchBuffer)
,	m_as(as)
{
}

}
