#include "Render/IRenderSystem.h"
#include "Render/Context/RenderBlock.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Frame/RenderPassBuilder.h"

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

bool RenderGraph::addRenderTarget(handle_t targetId, const RenderTargetSetCreateDesc& rtscd, const RenderTargetAutoSize& rtas)
{
	if (m_targets.find(targetId) != m_targets.end())
		return false;
	m_targets[targetId].rtscd = rtscd;
	m_targets[targetId].rtas = rtas;
	return true;
}

IRenderTargetSet* RenderGraph::getRenderTarget(handle_t targetId) const
{
	auto it = m_targets.find(targetId);
	if (it != m_targets.end())
		return it->second.rts[0];
	else
		return nullptr;
}

void RenderGraph::addPass(const RenderPass::fn_setup_t& setup, const RenderPass::fn_build_t& build)
{
	// Allocate pass from list.
	auto& pass = m_passes.push_back();

	// Setup pass using builder.
	RenderPassBuilder builder(pass);
	setup(builder);

	// Attach build handler.
	pass.m_build = build;
}

bool RenderGraph::validate()
{
	for (auto& tm : m_targets)
	{
		if (tm.second.rts[0])
			continue;

		RenderTargetSetCreateDesc rtscd = tm.second.rtscd;
		const auto& rtas = tm.second.rtas;

		if (rtas.screenWidthDenom > 0)
			rtscd.width = (m_width + rtas.screenWidthDenom - 1) / rtas.screenWidthDenom;
		if (rtas.screenHeightDenom > 0)
			rtscd.height = (m_height + rtas.screenHeightDenom - 1) / rtas.screenHeightDenom;

		if (rtas.maxWidth > 0)
			rtscd.width = min< int32_t >(rtscd.width, rtas.maxWidth);
		if (rtas.maxHeight > 0)
			rtscd.height = min< int32_t >(rtscd.height, rtas.maxHeight);

		tm.second.rts[0] = m_renderSystem->createRenderTargetSet(rtscd, T_FILE_LINE_W);
		if (!tm.second.rts[0])
			return false;

		bool cyclic = false;
		for (const auto& pass : m_passes)
		{
			if (pass.m_output.targetSetName != tm.first)
				continue;
			auto it = std::find_if(pass.m_inputs.begin(), pass.m_inputs.end(), [&](const RenderPass::Input& input) {
				return input.targetSetName == tm.first;
			});
			cyclic |= (bool)(it != pass.m_inputs.end());
		}
		if (cyclic)
		{
			tm.second.rts[1] = m_renderSystem->createRenderTargetSet(rtscd, T_FILE_LINE_W);
			if (!tm.second.rts[1])
				return false;
		}
	}

	// Collect all "root" passes, ie passes which doesn't require any inputs.
	for (size_t i = 0; i < m_passes.size(); ++i)
	{
		const auto& pass = m_passes[i];
		if (pass.m_inputs.empty())
			m_order.push_back(i);
	}

	// Add passes which has all it's inputs already queued in order.
	while (m_order.size() != m_passes.size())
	{
		bool found = false;
		for (size_t i = 0; i < m_passes.size(); ++i)
		{
			const auto& pass = m_passes[i];
			if (pass.m_inputs.empty())
				continue;

			size_t satisfied = 0;
			for (auto index : m_order)
			{
				for (uint32_t j =  0; j < pass.m_inputs.size(); ++j)
				{
					if (pass.m_inputs[j].targetSetName == m_passes[index].m_output.targetSetName)
						satisfied |= 1 << j;
				}
			}
			if (satisfied == pass.m_inputs.size() - 1)
			{
				m_order.push_back(i);
				found = true;
			}
		}
		if (!found)
		{
			// Unable to satisfy atleast a single pass this iteration.
			return false;
		}
	}

	return true;
}

bool RenderGraph::build(RenderContext* renderContext)
{
	T_ASSERT (m_width >= 0 && m_height >= 0);

	auto tb = renderContext->alloc< TargetBeginRenderBlock >();
	tb->renderTargetSet = nullptr;
	tb->clear.mask = render::CfColor | render::CfDepth | render::CfStencil;
	tb->clear.colors[0] = Color4f(46 / 255.0f, 56 / 255.0f, 92 / 255.0f, 1.0f);
	tb->clear.depth = 1.0f;
	tb->clear.stencil = 0;
	renderContext->enqueue(tb);

	for (auto index : m_order)
	{
		const auto& pass = m_passes[index];

		if (pass.m_output.targetSetName != 0)
		{
			auto it = m_targets.find(pass.m_output.targetSetName);
			T_ASSERT(it != m_targets.end());

			auto tb = renderContext->alloc< TargetBeginRenderBlock >();

			if (it->second.rts[1])
				tb->renderTargetSet = it->second.rts[1];
			else
				tb->renderTargetSet = it->second.rts[0];

			tb->renderTargetIndex = pass.m_output.targetColorIndex;
			tb->clear = pass.m_output.clear;
			renderContext->enqueue(tb);			
		}

		if (pass.m_build)
		{
			RenderPassResources resources(this);
			pass.m_build(resources, renderContext);
		}

		if (pass.m_output.targetSetName != 0)
		{
			auto it = m_targets.find(pass.m_output.targetSetName);
			T_ASSERT(it != m_targets.end());

			auto te = renderContext->alloc< TargetEndRenderBlock >();
			renderContext->enqueue(te);

			if (it->second.rts[0] && it->second.rts[1])
			{
				std::swap(
					it->second.rts[0],
					it->second.rts[1]
				);
			}
		}
	}

	auto te = renderContext->alloc< TargetEndRenderBlock >();
	renderContext->enqueue(te);

	auto p = renderContext->alloc< PresentRenderBlock >();
	renderContext->enqueue(p);

	return true;
}

	}
}