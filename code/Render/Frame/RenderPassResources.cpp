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
#if defined(_DEBUG)
    auto it = std::find_if(m_pass.m_inputs.begin(), m_pass.m_inputs.end(), [&](const RenderPass::Input& input) {
        return input.targetSetName == targetId;
    });
    T_ASSERT(it != m_pass.m_inputs.end());
#endif
    return m_renderGraph->getRenderTarget(targetId);
}

    }
}