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

namespace traktor::render
{

class ApiBuffer;
class Buffer;
class BufferDynamicVk;
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

	static Ref< AccelerationStructureVk > createTopLevel(Context* context, uint32_t numInstances, uint32_t inFlightCount);

	static Ref< AccelerationStructureVk > createBottomLevel(Context* context, const Buffer* vertexBuffer, const IVertexLayout* vertexLayout, const Buffer* indexBuffer, IndexType indexType, const AlignedVector< RaytracingPrimitives >& primitives, bool dynamic);

	virtual void destroy() override final;

	bool writeInstances(CommandBuffer* commandBuffer, const AlignedVector< Instance >& instances);

	bool writeGeometry(CommandBuffer* commandBuffer, const IBufferView* vertexBuffer, const IVertexLayout* vertexLayout, const IBufferView* indexBuffer, IndexType indexType, const AlignedVector< RaytracingPrimitives >& primitives, bool rebuild);

	const VkAccelerationStructureKHR& getVkAccelerationStructureKHR() const { return m_as[m_index]; }

protected:
	Context* m_context = nullptr;
	Ref< BufferDynamicVk > m_instanceBuffer;
	// Ring of acceleration structures. A dynamic bottom level structure is rebuilt every
	// frame on the asynchronous compute queue while prior frames' graphics ray queries
	// still read it, so it is buffered to the in-flight count; each frame writes a fresh
	// slot. Top level and static bottom level structures use a single slot.
	AlignedVector< Ref< ApiBuffer > > m_hierarchyBuffers;
	AlignedVector< Ref< ApiBuffer > > m_scratchBuffers;
	AlignedVector< VkAccelerationStructureKHR > m_as;
	uint32_t m_index = 0;
	uint32_t m_scratchAlignment = 0;
	bool m_dynamic = false;

	explicit AccelerationStructureVk(Context* context, bool dynamic);
};

}
