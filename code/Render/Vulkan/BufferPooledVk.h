/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Ref.h"
#include "Render/Vulkan/BufferViewVk.h"
#include "Render/Vulkan/BufferVk.h"

namespace traktor::render
{

class ApiBuffer;

/*! Buffer backed by a pool of allocations.
 * \ingroup Render
 *
 * Retired allocations are reused once every frame which might read them has been consumed.
 */
class BufferPooledVk : public BufferVk
{
	T_RTTI_CLASS;

public:
	explicit BufferPooledVk(Context* context, uint32_t bufferSize, uint32_t& instances);

	virtual ~BufferPooledVk();

	bool create(uint32_t usageBits);

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void unlock() override final;

	virtual const IBufferView* getBufferView() const override final;

	virtual void nextFrame() override final;

private:
	struct Allocation
	{
		Ref< ApiBuffer > buffer;
		BufferViewVk bufferView;
		uint64_t retiredFrame = 0; //!< Number of frames submitted when retired.
	};

	Allocation* m_current = nullptr;
	AlignedVector< Allocation* > m_retired; //!< In order of retirement.
	Ref< ApiBuffer > m_stageBuffer;
	uint32_t m_usageBits = 0;

	Allocation* createAllocation();

	bool isFree(const Allocation* allocation, uint64_t submittedFrames) const;
};

}
