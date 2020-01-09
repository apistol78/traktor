#include "Render/IRenderSystem.h"
#include "Render/Context/RenderBlock.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Frame/RenderPass.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderGraph", RenderGraph, IRenderGraph)

RenderGraph::RenderGraph(IRenderSystem* renderSystem)
:	m_renderSystem(renderSystem)
,	m_width(-1)
,	m_height(-1)
{
}

bool RenderGraph::addRenderTarget(const std::wstring& targetId, const RenderTargetSetCreateDesc& rtscd, const RenderTargetAutoSize& rtas)
{
	if (m_targets.find(targetId) != m_targets.end())
		return false;

	m_targets[targetId].rtscd = rtscd;
	m_targets[targetId].rtas = rtas;
	m_targets[targetId].rts = nullptr;
	return true;
}

IRenderTargetSet* RenderGraph::getRenderTarget(const std::wstring& targetId) const
{
	auto it = m_targets.find(targetId);
	if (it != m_targets.end())
		return it->second.rts;
	else
		return nullptr;
}

RenderPass* RenderGraph::addRenderPass(const std::wstring& passId)
{
	Ref< RenderPass > renderPass = new RenderPass(passId);
	m_passes.push_back(renderPass);
	m_order.resize(0);
	return renderPass;
}

void RenderGraph::removeRenderPass(const std::wstring& passId)
{
	auto it = std::find_if(m_passes.begin(), m_passes.end(), [&](const RenderPass* renderPass) {
		return renderPass->m_passId == passId;
	});
	if (it != m_passes.end())
	{
		m_passes.erase(it);
		m_order.resize(0);
	}
}

bool RenderGraph::validate(int32_t width, int32_t height)
{
	// Always assume graph is valid if it doesn't contain any pass.
	if (m_passes.empty())
		return true;

	// Check if we need to re-create targets and/or passes.
	if (
		!m_order.empty() &&
		m_width == width &&
		m_height == height
	)
		return true;

	// Create render targets.
	for (auto& tm : m_targets)
	{
		RenderTargetSetCreateDesc rtscd = tm.second.rtscd;
		const auto& rtas = tm.second.rtas;

		if (rtas.screenWidthDenom > 0)
			rtscd.width = (width + rtas.screenWidthDenom - 1) / rtas.screenWidthDenom;
		if (rtas.screenHeightDenom > 0)
			rtscd.height = (height + rtas.screenHeightDenom - 1) / rtas.screenHeightDenom;

		if (rtas.maxWidth > 0)
			rtscd.width = min< int32_t >(rtscd.width, rtas.maxWidth);
		if (rtas.maxHeight > 0)
			rtscd.height = min< int32_t >(rtscd.height, rtas.maxHeight);

		tm.second.rts = m_renderSystem->createRenderTargetSet(rtscd, tm.first.c_str());
		if (!tm.second.rts)
			return false;
	}

	// Collect all "root" passes, ie passes which doesn't require any inputs.
	for (size_t i = 0; i < m_passes.size(); ++i)
	{
		auto pass = m_passes[i];
		if (pass->m_inputs.empty())
			m_order.push_back(i);
	}

	// Add passes which has all it's inputs already queued in order.
	while (m_order.size() != m_passes.size())
	{
		bool found = false;
		for (size_t i = 0; i < m_passes.size(); ++i)
		{
			auto pass = m_passes[i];
			if (pass->m_inputs.empty())
				continue;

			size_t satisfied = 0;
			for (auto index : m_order)
			{
				for (uint32_t j =  0; j < pass->m_inputs.size(); ++j)
				{
					if (pass->m_inputs[j].targetSetName == m_passes[index]->m_output)
						satisfied |= 1 << j;
				}
			}
			if (satisfied == pass->m_inputs.size() - 1)
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

	m_width = width;
	m_height = height;
	return true;
}

bool RenderGraph::render(RenderContext* renderContext)
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
		auto pass = m_passes[index];
		T_FATAL_ASSERT(pass != nullptr);

		if (!pass->m_output.empty())
		{
			auto it = m_targets.find(pass->m_output);
			T_ASSERT(it != m_targets.end());

			auto tb = renderContext->alloc< TargetBeginRenderBlock >();
			tb->renderTargetSet = it->second.rts;
			renderContext->enqueue(tb);			
		}

		if (pass->m_handler)
			pass->m_handler->executeRenderPass(this, renderContext);

		if (!pass->m_output.empty())
		{
			auto te = renderContext->alloc< TargetEndRenderBlock >();
			renderContext->enqueue(te);
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