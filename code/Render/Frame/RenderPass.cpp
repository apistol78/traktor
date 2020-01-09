#include "Render/Frame/RenderPass.h"

namespace traktor
{
    namespace render
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderPass", RenderPass, Object)

void RenderPass::addInput(const std::wstring& targetSetName, uint32_t targetColorIndex)
{
    m_inputs.push_back({ targetSetName, targetColorIndex });
}

uint32_t RenderPass::getInputCount() const
{
    return (uint32_t)m_inputs.size();
}

void RenderPass::setOutput(const std::wstring& targetSetName)
{
    m_output = targetSetName;
}

void RenderPass::setHandler(IHandler* handler)
{
    m_handler = handler;
}

RenderPass::RenderPass(const std::wstring& passId)
:   m_passId(passId)
{
}

RenderPass::LambdaHandler::LambdaHandler(fn_t fn)
:   m_fn(fn)
{
}

void RenderPass::LambdaHandler::executeRenderPass(IRenderGraph* renderGraph, RenderContext* renderContext)
{
    m_fn(renderGraph, renderContext);
}

    }
}
