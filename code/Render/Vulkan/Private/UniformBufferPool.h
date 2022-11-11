/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Render/Vulkan/Private/UniformBufferChain.h"

namespace traktor::render
{

class UniformBufferPool : public Object
{
public:
	void destroy();

	void recycle();

	void flush();

	bool allocate(uint32_t size, UniformBufferRange& outRange);

	void free(const UniformBufferRange& range);

private:
	friend class Context;

	const wchar_t* const m_name;
	Context* m_context = nullptr;
	SmallMap< uint32_t, RefArray< UniformBufferChain > > m_chains;	//!< Map of chains; map size of block to chain.
	AlignedVector< UniformBufferRange > m_frees[16];
	uint32_t m_blockCount = 0;
	uint32_t m_count = 0;

	explicit UniformBufferPool(Context* context, uint32_t blockCount, const wchar_t* const name);
};

}
