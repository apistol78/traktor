/*
 * TRAKTOR
 * Copyright (c) 2024-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Ref.h"
#include "Render/IAccelerationStructure.h"
#include "Render/Types.h"
#include "Render/Vulkan/Private/ApiHeader.h"

#include <atomic>

namespace traktor::render
{

class ApiBuffer;
class Buffer;
class CommandBuffer;
class Context;
class IBufferView;
class IVertexLayout;

/*!
 * \ingroup Render
 */
class AccelerationStructureVk : public IAccelerationStructure
{
	T_RTTI_CLASS;

public:
	virtual ~AccelerationStructureVk();

	static Ref< AccelerationStructureVk > createTopLevel(Context* context, uint32_t numInstances, bool pooled);

	static Ref< AccelerationStructureVk > createBottomLevel(Context* context, const Buffer* vertexBuffer, const IVertexLayout* vertexLayout, const Buffer* indexBuffer, IndexType indexType, const AlignedVector< RaytracingPrimitives >& primitives, bool dynamic, bool pooled);

	virtual void destroy() override final;

	virtual void nextFrame() override final;

	bool writeInstances(CommandBuffer* commandBuffer, const AlignedVector< Instance >& instances);

	bool writeGeometry(CommandBuffer* commandBuffer, const IBufferView* vertexBuffer, const IVertexLayout* vertexLayout, const IBufferView* indexBuffer, IndexType indexType, const AlignedVector< RaytracingPrimitives >& primitives, bool rebuild);

	const VkAccelerationStructureKHR& getVkAccelerationStructureKHR() const { return m_as[m_index]; }

protected:
	//! Use of a ring slot.
	struct Slot
	{
		bool used = false;	//!< Slot has been current, thus submitted work might read it.
		uint64_t leftFrame = 0;	//!< Number of frames submitted when the slot stopped being current.
	};

	Context* m_context = nullptr;
	AlignedVector< Ref< ApiBuffer > > m_instanceBuffers;	//!< Top level only; instance data read by the build of each slot.
	AlignedVector< Ref< ApiBuffer > > m_hierarchyBuffers;
	AlignedVector< Ref< ApiBuffer > > m_scratchBuffers;
	AlignedVector< VkAccelerationStructureKHR > m_as;
	AlignedVector< Slot > m_slots;
	uint32_t m_index = 0;
	uint32_t m_scratchAlignment = 0;
	uint32_t m_instanceCapacity = 0;	//!< Top level only; number of instances each slot holds.
	VkDeviceSize m_topLevelSize = 0;	//!< Top level only; size of each slot's structure.
	VkDeviceSize m_topLevelScratchSize = 0;	//!< Top level only; size of each slot's build scratch.
	std::atomic< uint32_t > m_pendingFrames = 0;	//!< Frames begun by nextFrame whose write has not been rendered.
	bool m_topLevel = false;
	bool m_dynamic = false;
	bool m_pooled = false;

	explicit AccelerationStructureVk(Context* context, bool topLevel, bool dynamic, bool pooled);

private:
	struct GeometryBuild
	{
		AlignedVector< VkAccelerationStructureGeometryKHR > geometries;
		AlignedVector< VkAccelerationStructureBuildRangeInfoKHR > ranges;
		VkAccelerationStructureBuildGeometryInfoKHR info;	//!< Geometries are referenced when recorded, as the build might have been copied since prepared.
	};

	void teardown();

	/*! Make the next ring slot current when a frame has begun. */
	void beginWrite();

	/*! Make the next ring slot current; the ring grows rather than rewrite a slot which might still be read. */
	void advance();

	/*! Check if a slot can be rewritten, i.e. no recorded or submitted work reads it any longer. */
	bool isSlotFree(uint32_t slot, uint64_t submittedFrames) const;

	/*! Insert an unused slot into the ring. */
	bool insertSlot(uint32_t at);

	/*! Create instance, hierarchy and scratch buffers and the structure of a top level slot. */
	bool createTopLevelSlot(uint32_t slot);

	/*! Prepare build of bottom level structure into the current slot, refit from source unless rebuilt; (re-)creates its buffers and structure as required. */
	bool prepareGeometry(const IBufferView* vertexBuffer, const IVertexLayout* vertexLayout, const IBufferView* indexBuffer, IndexType indexType, const AlignedVector< RaytracingPrimitives >& primitives, bool rebuild, VkAccelerationStructureKHR source, GeometryBuild& outBuild);

	/*! Record prepared build of bottom level structure. */
	static void recordGeometry(CommandBuffer* commandBuffer, const GeometryBuild& build);
};

}
