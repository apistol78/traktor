/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
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

/*!
 * \ingroup Render
 */
class BufferReadBackVk : public BufferVk
{
	T_RTTI_CLASS;

public:
	explicit BufferReadBackVk(Context* context, uint32_t bufferSize, uint32_t& instances);

	virtual ~BufferReadBackVk();

	bool create(uint32_t usageBits);

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void unlock() override final;

	virtual const IBufferView* getBufferView() const override final;

	const ApiBuffer* getApiBuffer() const { return m_buffer; }

private:
	void teardown();

	Ref< ApiBuffer > m_buffer;
	BufferViewVk m_bufferView;
	void* m_readBack = nullptr;
};

}
