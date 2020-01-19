#include "Core/Log/Log.h"
#include "Render/IRenderSystem.h"
#include "Render/Context/RenderBlock.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Frame/RenderPassBuilder.h"

namespace traktor
{
	namespace render
	{
		namespace
		{
		
void traverse(const AlignedVector< RenderPass >& passes, int32_t index)
{
	const auto& pass = passes[index];

	for (const auto& input : pass.m_inputs)
	{
		
	}
}

		}

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

void RenderGraph::addPass(const wchar_t* const name, const RenderPass::fn_setup_t& setup, const RenderPass::fn_build_t& build)
{
	// Allocate pass from list.
	auto& pass = m_passes.push_back();
	pass.m_name = name;

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

	for (const auto& consumer : m_passes)
	{
		for (const auto& input : consumer.m_inputs)
		{
			for (auto& producer : m_passes)
			{
				if (producer.m_output.targetSetName == input.targetSetName)
					producer.m_refs++;
			}
		}
	}

	// Collect all passes which write to framebuffer.
	for (size_t i = 0; i < m_passes.size(); ++i)
	{
		const auto& pass = m_passes[i];
		if (pass.m_output.targetSetName != 0)
			continue;

		m_order.push_back(i);
	}

	int32_t from = 0;
	for (;;)
	{
		int32_t to = (int32_t)m_order.size();
	
		for (int32_t i = from; i < to; ++i)
		{
			const auto& pass = m_passes[m_order[i]];
			for (const auto& input : pass.m_inputs)
			{
				for (int32_t j = 0; j < m_passes.size(); ++j)
				{
					if (
						m_passes[j].m_valid == false &&
						m_passes[j].m_output.targetSetName == input.targetSetName
					)
					{
						m_order.push_back(j);
						m_passes[j].m_valid = true;
						break;
					}
				}
			}
		}

		if (to == m_order.size())
			break;

		from = to;
	}

	std::reverse(m_order.begin(), m_order.end());

	log::info << L"== Render passes ==" << Endl;
	for (auto index : m_order)
		log::info << index << L". " << m_passes[index].m_name << Endl;
	log::info << L"===================" << Endl;

	return true;
}

bool RenderGraph::build(RenderContext* renderContext)
{
	T_ASSERT (m_width >= 0 && m_height >= 0);

	//auto tb = renderContext->alloc< TargetBeginRenderBlock >();
	//tb->renderTargetSet = nullptr;
	//tb->clear.mask = render::CfColor | render::CfDepth | render::CfStencil;
	//tb->clear.colors[0] = Color4f(46 / 255.0f, 56 / 255.0f, 92 / 255.0f, 1.0f);
	//tb->clear.depth = 1.0f;
	//tb->clear.stencil = 0;
	//renderContext->enqueue(tb);

	for (auto index : m_order)
	{
		const auto& pass = m_passes[index];

		if (pass.m_output.targetSetName != 0)
		{
			auto it = m_targets.find(pass.m_output.targetSetName);
			T_FATAL_ASSERT(it != m_targets.end());

			auto tb = renderContext->alloc< TargetBeginRenderBlock >();

			if (it->second.rts[1])
				tb->renderTargetSet = it->second.rts[1];
			else
				tb->renderTargetSet = it->second.rts[0];
			T_FATAL_ASSERT(tb->renderTargetSet != nullptr);

			tb->renderTargetIndex = pass.m_output.targetColorIndex;
			tb->clear = pass.m_output.clear;
			renderContext->enqueue(tb);			
		}

		if (pass.m_build)
		{
			RenderPassResources resources(this, pass);
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

	//auto te = renderContext->alloc< TargetEndRenderBlock >();
	//renderContext->enqueue(te);

	//auto p = renderContext->alloc< PresentRenderBlock >();
	//renderContext->enqueue(p);

	m_order.resize(0);
	m_passes.resize(0);

	return true;
}

	}
}