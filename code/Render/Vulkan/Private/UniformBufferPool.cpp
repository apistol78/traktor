/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Vulkan/Private/UniformBufferPool.h"

namespace traktor::render
{

void UniformBufferPool::destroy()
{
	flush();
}

void UniformBufferPool::recycle(uint64_t issuedEpoch, uint64_t completedEpoch)
{
	// Blocks freed since last time were used by commands submitted at or before the issued epoch.
	if (!m_freed.empty())
	{
		if (!m_retired.empty() && m_retired.back().epoch == issuedEpoch)
			m_retired.back().ranges.insert(m_retired.back().ranges.end(), m_freed.begin(), m_freed.end());
		else
		{
			auto& retired = m_retired.push_back();
			retired.epoch = issuedEpoch;
			retired.ranges.swap(m_freed);
		}
		m_freed.resize(0);
	}

	// Return blocks the GPU is done with.
	size_t count = 0;
	for (; count < m_retired.size(); ++count)
	{
		const Retired& retired = m_retired[count];
		if (retired.epoch > completedEpoch)
			break;
		for (const auto& range : retired.ranges)
			range.chain->free(range);
	}
	if (count > 0)
		m_retired.erase(m_retired.begin(), m_retired.begin() + count);
}

void UniformBufferPool::flush()
{
	m_freed.resize(0);
	m_retired.resize(0);

	SmallMap< uint32_t, RefArray< UniformBufferChain > > chains;
	chains.swap(m_chains);
	for (const auto& it : chains)
	{
		for (auto chain : it.second)
			chain->destroy();
	}
}

bool UniformBufferPool::allocate(uint32_t size, UniformBufferRange& outRange)
{
	auto& chains = m_chains[size];

	// Try to allocate from an existing chain.
	for (auto chain : chains)
	{
		if (chain->allocate(outRange))
			return true;
	}

	// No chain found which has a free block, create new chain and allocate from that.
	Ref< UniformBufferChain > chain = UniformBufferChain::create(m_context, m_blockCount, size);
	if (!chain)
		return false;

	chain->allocate(outRange);
	chains.push_back(chain);
	return true;
}

void UniformBufferPool::free(const UniformBufferRange& range)
{
	m_freed.push_back(range);
}

UniformBufferPool::UniformBufferPool(Context* context, uint32_t blockCount, const wchar_t* const name)
:	m_name(name)
,	m_context(context)
,	m_blockCount(blockCount)
{
}

}
