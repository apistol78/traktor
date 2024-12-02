/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Render/Vulkan/Private/ApiHeader.h"

namespace traktor::render
{

class Context;

/*!
 * \ingroup Render
 */
class ApiBuffer : public Object
{
	T_RTTI_CLASS;

public:
	ApiBuffer() = delete;

	ApiBuffer(const ApiBuffer&) = delete;

	ApiBuffer(ApiBuffer&&) = delete;

	explicit ApiBuffer(Context* context);

	virtual ~ApiBuffer();

	bool create(uint32_t bufferSize, uint32_t usageBits, bool cpuAccess, bool gpuAccess);

	void destroy();

	void* lock();

	void unlock();

	VkDeviceAddress getDeviceAddress();

	uint32_t makeResourceIndex() const;

	uint32_t getSize() const { return m_bufferSize; }

	operator VkBuffer () const { return m_buffer; }

private:
	Context* m_context = nullptr;
	VmaAllocation m_allocation = 0;
	VkBuffer m_buffer = 0;
	uint32_t m_bufferSize = 0;
	mutable uint32_t m_resourceIndex = ~0U;
	void* m_locked = nullptr;
};
		
}
