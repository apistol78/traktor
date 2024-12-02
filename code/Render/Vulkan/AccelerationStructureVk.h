/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
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

	static Ref< AccelerationStructureVk > createBottomLevel(Context* context, const Buffer* vertexBuffer, const IVertexLayout* vertexLayout, const Buffer* indexBuffer, IndexType indexType, const AlignedVector< Primitives >& primitives);

	virtual void destroy() override final;

	bool writeInstances(CommandBuffer* commandBuffer, const AlignedVector< Instance >& instances);

	bool writeGeometry(CommandBuffer* commandBuffer, const IBufferView* vertexBuffer, const IVertexLayout* vertexLayout, const IBufferView* indexBuffer, IndexType indexType, const AlignedVector< Primitives >& primitives);

	const VkAccelerationStructureKHR& getVkAccelerationStructureKHR() const { return m_as; }

protected:
	Context* m_context = nullptr;
	Ref< BufferDynamicVk > m_instanceBuffer;
	Ref< ApiBuffer > m_hierarchyBuffer;
	Ref< ApiBuffer > m_scratchBuffer;
	VkAccelerationStructureKHR m_as = 0;
	uint32_t m_scratchAlignment = 0;

	explicit AccelerationStructureVk(Context* context);
};

}
