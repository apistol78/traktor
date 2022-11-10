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

class ApiBuffer;

class BufferStaticVk : public BufferVk
{
	T_RTTI_CLASS;

public:
	explicit BufferStaticVk(Context* context, uint32_t elementCount, uint32_t elementSize, uint32_t& instances);

	virtual ~BufferStaticVk();

	bool create(uint32_t usageBits);

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void unlock() override final;

	virtual const IBufferView* getBufferView() const override final;

private:
	Ref< ApiBuffer > m_buffer;
	Ref< ApiBuffer > m_stageBuffer;
	BufferViewVk m_bufferView;
	uint32_t m_size = 0;
};

}
