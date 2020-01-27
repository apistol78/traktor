#include "Render/Frame/RenderGraph.h"
#include "Render/Frame/RenderPassResources.h"

namespace traktor
{
    namespace render
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderPassResources", RenderPassResources, Object)

RenderPassResources::RenderPassResources(const RenderGraph* renderGraph, const RenderPass& pass)
:   m_renderGraph(renderGraph)
,   m_pass(pass)
{
}

IRenderTargetSet* RenderPassResources::getInput(handle_t targetId) const
{
    auto it = std::find_if(m_pass.m_inputs.begin(), m_pass.m_inputs.end(), [&](const RenderPass::Input& input) {
        return input.name == targetId;
    });
    if (it != m_pass.m_inputs.end())
        return m_renderGraph->getRenderTarget(targetId, it->history);
    else
        return nullptr;
}

    }
}