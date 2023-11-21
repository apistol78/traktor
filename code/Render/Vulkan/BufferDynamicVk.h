/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Render/Vulkan/BufferViewVk.h"
#include "Render/Vulkan/BufferVk.h"
#include "Render/Vulkan/Private/ApiBuffer.h"

namespace traktor::render
{

class BufferDynamicVk : public BufferVk
{
	T_RTTI_CLASS;

public:
	explicit BufferDynamicVk(Context* context, uint32_t bufferSize, uint32_t& instances);

	virtual ~BufferDynamicVk();

	bool create(uint32_t usageBits, int32_t inFlightCount);

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void unlock() override final;

	virtual const IBufferView* getBufferView() const override final;

private:
	Ref< ApiBuffer > m_buffer;
	BufferViewVk* m_bufferViews = nullptr;
	uint32_t m_inFlightCount = 0;
	uint32_t m_range = 0;
	uint32_t m_index = 0;
	uint32_t m_view = 0;
	uint8_t* m_ptr = nullptr;
};

}
