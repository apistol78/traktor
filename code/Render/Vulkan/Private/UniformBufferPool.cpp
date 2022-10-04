#include "Render/Vulkan/Private/UniformBufferPool.h"

namespace traktor::render
{

void UniformBufferPool::destroy()
{
	flush();
}

void UniformBufferPool::recycle()
{
	const uint32_t idx = (m_count - sizeof_array(m_frees) + 1) % sizeof_array(m_frees);

	auto& frees = m_frees[idx];
	for (auto free : frees)
		free.chain->free(free);
	frees.resize(0);

	m_count++;
}

void UniformBufferPool::flush()
{
	for (int32_t i = 0; i < sizeof_array(m_frees); ++i)
		m_frees[i].resize(0);

	SmallMap< uint32_t, RefArray< UniformBufferChain > > chains;
	chains.swap(m_chains);
	for (auto it : chains)
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
	const uint32_t idx = m_count % sizeof_array(m_frees);
	m_frees[idx].push_back(range);
}

UniformBufferPool::UniformBufferPool(Context* context, uint32_t blockCount, const wchar_t* const name)
:	m_name(name)
,	m_context(context)
,	m_blockCount(blockCount)
{
}

}
