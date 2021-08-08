#include "Render/Vulkan/Private/UniformBufferPool.h"

namespace traktor
{
	namespace render
	{

void UniformBufferPool::destroy()
{
	for (auto it : m_chains)
	{
		for (auto chain : it.second)
			chain->destroy();
	}
	m_chains.clear();
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
	Ref< UniformBufferChain > chain = UniformBufferChain::create(m_context, 1024, size);
	if (!chain)
		return nullptr;

	chain->allocate(outRange);
	chains.push_back(chain);

	return true;
}

void UniformBufferPool::free(const UniformBufferRange& range)
{
	const uint32_t idx = m_count % sizeof_array(m_frees);
	m_frees[idx].push_back(range);
}

UniformBufferPool::UniformBufferPool(Context* context)
:	m_context(context)
{
}

	}
}
