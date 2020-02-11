#include "Core/Containers/SmallSet.h"
#include "Core/Log/Log.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/Context/RenderBlock.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"

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
}

void RenderGraph::destroy()
{
	m_renderSystem = nullptr;
	m_targets.clear();
	m_passes.clear();
	m_order.clear();
}

bool RenderGraph::addTargetSet(
	handle_t targetSetId,
	const RenderGraphTargetSetDesc& targetSetDesc,
	IRenderTargetSet* sharedDepthStencilTargetSet
)
{
	if (m_targets.find(targetSetId) != m_targets.end())
		return false;
	m_targets[targetSetId].targetSetDesc = targetSetDesc;
	m_targets[targetSetId].sharedDepthStencilTargetSet = sharedDepthStencilTargetSet;
	return true;
}

IRenderTargetSet* RenderGraph::getTargetSet(handle_t targetSetId, bool history) const
{
	auto it = m_targets.find(targetSetId);
	if (it != m_targets.end())
		return it->second.rts[history ? 1 : 0];
	else
		return nullptr;
}

void RenderGraph::addPass(const RenderPass* pass)
{
	m_passes.push_back(pass);
}

bool RenderGraph::validate()
{
	// Create targets.
	for (auto& tm : m_targets)
	{
		if (tm.second.rts[0])
			continue;

		const auto& td = tm.second.targetSetDesc;

		RenderTargetSetCreateDesc rtscd = {};
		rtscd.count = td.count;
		rtscd.width = td.width;
		rtscd.height = td.height;
		rtscd.multiSample = 0;
		rtscd.createDepthStencil = td.createDepthStencil;
		rtscd.usingPrimaryDepthStencil = td.usingPrimaryDepthStencil;
		rtscd.usingDepthStencilAsTexture = td.usingDepthStencilAsTexture;
		rtscd.storeDepthStencil = true;
		rtscd.ignoreStencil = false;
		rtscd.generateMips = td.generateMips;

		for (int32_t i = 0; i < td.count; ++i)
			rtscd.targets[i].format = td.targets[i].colorFormat;
		
		if (td.screenWidthDenom > 0)
			rtscd.width = (m_width + td.screenWidthDenom - 1) / td.screenWidthDenom;
		if (td.screenHeightDenom > 0)
			rtscd.height = (m_height + td.screenHeightDenom - 1) / td.screenHeightDenom;
		if (td.maxWidth > 0)
			rtscd.width = min< int32_t >(rtscd.width, td.maxWidth);
		if (td.maxHeight > 0)
			rtscd.height = min< int32_t >(rtscd.height, td.maxHeight);

		tm.second.rts[0] = m_renderSystem->createRenderTargetSet(rtscd, tm.second.sharedDepthStencilTargetSet, T_FILE_LINE_W);
		if (!tm.second.rts[0])
			return false;

		// Check if any pass require history of this target.
		bool needHistory = false;
		for (auto pass : m_passes)
		{
			for (const auto& input : pass->getInputs())
			{
				if (input.targetSetId == tm.first)
					needHistory |= input.history;
			}
		}
		if (needHistory)
		{
			tm.second.rts[1] = m_renderSystem->createRenderTargetSet(rtscd, nullptr, T_FILE_LINE_W);
			if (!tm.second.rts[1])
				return false;
		}
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
			tb->renderTargetSet = it->second.rts[0];
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

	// Keep target history.
	for (auto& target : m_targets)
	{
		if (target.second.rts[1])
			std::swap(target.second.rts[0], target.second.rts[1]);
	}

	// Remove all passes.
	m_order.resize(0);
	m_passes.resize(0);
	return true;
}

void RenderGraph::getDebugTargets(std::vector< render::DebugTarget >& outTargets) const
{
	for (auto it : m_targets)
	{
		if (it.second.rts[0])
			outTargets.push_back(render::DebugTarget(
				getParameterName(it.first),
				render::DtvDefault,
				it.second.rts[0]->getColorTexture(0)
			));

		if (it.second.rts[1])
			outTargets.push_back(render::DebugTarget(
				getParameterName(it.first),
				render::DtvDefault,
				it.second.rts[1]->getColorTexture(0)
			));			
	}
}

void RenderGraph::traverse(int32_t index, const std::function< void(int32_t) >& fn) const
{
	for (const auto& input : m_passes[index]->getInputs())
	{
		// History inputs are not included since they usually indicate a cyclic dependency.
		if (input.history)
			continue;

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