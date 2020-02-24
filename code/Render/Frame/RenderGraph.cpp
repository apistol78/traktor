#include "Core/Containers/StaticSet.h"
#include "Core/Log/Log.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/Context/RenderBlock.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Frame/RenderGraphTargetSetPool.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderGraph", RenderGraph, Object)

RenderGraph::RenderGraph(IRenderSystem* renderSystem)
:	m_nextTargetSetId(1)
{
	m_pool = new RenderGraphTargetSetPool(renderSystem);
}

void RenderGraph::destroy()
{
	m_targets.clear();
	m_passes.clear();
	m_order.clear();
	m_pool = nullptr;
}

handle_t RenderGraph::addTargetSet(
	const RenderGraphTargetSetDesc& targetSetDesc,
	IRenderTargetSet* sharedDepthStencilTargetSet,
	handle_t sizeReferenceTargetSetId
)
{
	handle_t targetSetId = m_nextTargetSetId++;
	auto& target = m_targets[targetSetId];
	target.targetSetDesc = targetSetDesc;
	target.sharedDepthStencilTargetSet = sharedDepthStencilTargetSet;
	target.sizeReferenceTargetSetId = sizeReferenceTargetSetId;
	return targetSetId;
}

IRenderTargetSet* RenderGraph::getTargetSet(handle_t targetSetId) const
{
	auto it = m_targets.find(targetSetId);
	if (it != m_targets.end())
		return it->second.rts;
	else
		return nullptr;
}

void RenderGraph::addPass(const RenderPass* pass)
{
	m_passes.push_back(pass);
}

bool RenderGraph::validate(int32_t width, int32_t height)
{
	// Acquire targets.
	for (auto& tm : m_targets)
	{
		int32_t referenceWidth = width;
		int32_t referenceHeight = height;

		if (tm.second.sizeReferenceTargetSetId != 0)
		{
			auto it = m_targets.find(tm.second.sizeReferenceTargetSetId);
			if (it == m_targets.end())
				return false;

			referenceWidth = it->second.targetSetDesc.width;
			referenceHeight = it->second.targetSetDesc.height;
		}

		tm.second.rts = m_pool->acquire(
			tm.second.targetSetDesc,
			tm.second.sharedDepthStencilTargetSet,
			referenceWidth,
			referenceHeight
		);
		if (!tm.second.rts)
			return false;
	}

	// Cleanup non-acquired targets.
	m_pool->cleanup();

	// Append passes depth-first.
	StaticVector< uint32_t, 256 > order;
	for (int32_t i = 0; i < (int32_t)m_passes.size(); ++i)
	{
		if (m_passes[i]->getOutput().targetSetId == 0)
		{
			traverse(i, [&](int32_t index) {
				order.push_back(index);
			});
		}
	}

	m_order.resize(0);

	StaticSet< uint32_t, 256 > added;
	for (auto index : order)
	{
		if (added.insert(index))
			m_order.push_back(index);
	}

#if defined(_DEBUG)
	for (int32_t i = 0; i < (int32_t)m_order.size(); ++i)
	{
		const auto pass = m_passes[m_order[i]];
		log::info << i << L". " << pass->getName() << L" -> " << pass->getOutput().targetSetId << Endl;
	}
#endif
	return true;
}

bool RenderGraph::build(RenderContext* renderContext)
{
	T_FATAL_ASSERT(!renderContext->havePendingDraws());

	// Render passes in dependency order.
	for (auto index : m_order)
	{
		const auto pass = m_passes[index];
		const auto& output = pass->getOutput();

		// Begin render pass.
		if (output.targetSetId != 0)
		{
			auto it = m_targets.find(output.targetSetId);
			T_FATAL_ASSERT(it != m_targets.end());

			auto tb = renderContext->alloc< TargetBeginRenderBlock >(std::wstring(L"Begin ") + pass->getName());
			tb->renderTargetSet = it->second.rts;
			tb->clear = output.clear;
			renderContext->enqueue(tb);			
		}

		// Build sub passes first.
		for (auto subPass : pass->getSubPasses())
		{
			for (const auto& build : subPass->getBuilds())
			{
				build(*this, renderContext);
				T_FATAL_ASSERT(!renderContext->havePendingDraws());
			}
		}

		// Build this pass.
		for (const auto& build : pass->getBuilds())
		{
			build(*this, renderContext);
			T_FATAL_ASSERT(!renderContext->havePendingDraws());
		}

		// End render pass.
		if (output.targetSetId != 0)
		{
			auto te = renderContext->alloc< TargetEndRenderBlock >(std::wstring(L"End ") + pass->getName());
			renderContext->enqueue(te);
		}
	}

	T_FATAL_ASSERT(!renderContext->havePendingDraws());

	// Release targets.
	for (auto& tm : m_targets)
	{
		if (tm.second.rts)
		{
			m_pool->release(tm.second.rts);
			tm.second.rts = nullptr;
		}
	}

	// Remove all data; keep memory allocated for arrays
	// since it's very likely this will be identically
	// re-populated next frame.
	m_targets.reset();
	m_passes.resize(0);
	m_order.resize(0);
	return true;
}

void RenderGraph::getDebugTargets(std::vector< render::DebugTarget >& outTargets) const
{
	for (auto it : m_targets)
	{
		if (it.second.rts)
			outTargets.push_back(render::DebugTarget(
				getParameterName(it.first),
				render::DtvDefault,
				it.second.rts->getColorTexture(0)
			));
	}
}

void RenderGraph::traverse(int32_t index, const std::function< void(int32_t) >& fn) const
{
	for (const auto& input : m_passes[index]->getInputs())
	{
		for (int32_t i = 0; i < m_passes.size(); ++i)
		{
			if (i == index)
				continue;
			if (m_passes[i]->getOutput().targetSetId == input.targetSetId)
				traverse(i, fn);
		}
	}
	fn(index);
}

	}
}