#include "Core/Containers/SmallSet.h"
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

RenderGraph::RenderGraph(IRenderSystem* renderSystem, int32_t width, int32_t height)
:	m_renderSystem(renderSystem)
,	m_width(width)
,	m_height(height)
{
	m_pool = new RenderGraphTargetSetPool(renderSystem, width, height);
}

void RenderGraph::destroy()
{
	m_renderSystem = nullptr;
	m_targets.clear();
	m_passes.clear();
	m_order.clear();
	m_pool = nullptr;
}

void RenderGraph::addTargetSet(
	handle_t targetSetId,
	const RenderGraphTargetSetDesc& targetSetDesc,
	IRenderTargetSet* sharedDepthStencilTargetSet
)
{
	auto& target = m_targets[targetSetId];
	target.targetSetDesc = targetSetDesc;
	target.sharedDepthStencilTargetSet = sharedDepthStencilTargetSet;
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

bool RenderGraph::validate()
{
	// Acquire targets.
	for (auto& tm : m_targets)
	{
		tm.second.rts = m_pool->acquire(tm.second.targetSetDesc, tm.second.sharedDepthStencilTargetSet);
		if (!tm.second.rts)
			return false;
	}

	// Append passes depth-first.
	SmallSet< uint32_t > order;
	for (int32_t i = 0; i < (int32_t)m_passes.size(); ++i)
	{
		if (m_passes[i]->getOutput().targetSetId == 0)
		{
			traverse(i, [&](int32_t index) {
				order.insert(index);
			});
		}
	}

	m_order.resize(0);
	m_order.insert(m_order.end(), order.begin(), order.end());
	return true;
}

bool RenderGraph::build(RenderContext* renderContext)
{
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

			auto tb = renderContext->alloc< TargetBeginRenderBlock >();
			tb->renderTargetSet = it->second.rts;
			tb->clear = output.clear;
			renderContext->enqueue(tb);			
		}

		// Build sub passes first.
		for (auto subPass : pass->getSubPasses())
		{
			for (const auto& build : subPass->getBuilds())
				build(*this, renderContext);
		}

		// Build this pass.
		for (const auto& build : pass->getBuilds())
			build(*this, renderContext);

		// End render pass.
		if (output.targetSetId != 0)
		{
			auto te = renderContext->alloc< TargetEndRenderBlock >();
			renderContext->enqueue(te);
		}
	}

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