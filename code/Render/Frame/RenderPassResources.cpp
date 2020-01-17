#include "Render/Frame/RenderGraph.h"
#include "Render/Frame/RenderPassResources.h"

namespace traktor
{
    namespace render
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderPassResources", RenderPassResources, Object)

RenderPassResources::RenderPassResources(const RenderGraph* renderGraph)
:   m_renderGraph(renderGraph)
{
}

IRenderTargetSet* RenderPassResources::getInput(handle_t targetId) const
{
    return m_renderGraph->getRenderTarget(targetId);
}

    }
}