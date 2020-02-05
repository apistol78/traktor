#include "Render/Frame/RenderPass.h"

namespace traktor
{
    namespace render
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderPass", RenderPass, Object)

RenderPass::RenderPass(const wchar_t* const name)
:   m_name(name)
{
}

void RenderPass::addInput(handle_t targetSetId)
{
    auto& input = m_inputs.push_back();
    input.targetSetId = targetSetId;
    input.colorIndex = 0;
    input.history = false;
}

void RenderPass::addInput(handle_t targetSetId, int32_t colorIndex, bool history)
{
    auto& input = m_inputs.push_back();
    input.targetSetId = targetSetId;
    input.colorIndex = colorIndex;
    input.history = history;
}

void RenderPass::setOutput(handle_t targetSetId)
{
    m_output.targetSetId = targetSetId;
    m_output.colorIndex = -1;
    m_output.clear.mask = 0;
}

void RenderPass::setOutput(handle_t targetSetId, const Clear& clear)
{
    m_output.targetSetId = targetSetId;
    m_output.colorIndex = -1;
    m_output.clear = clear;
}

void RenderPass::setOutput(handle_t targetSetId, int32_t colorIndex, const Clear& clear)
{
    m_output.targetSetId = targetSetId;
    m_output.colorIndex = colorIndex;
    m_output.clear = clear;
}

void RenderPass::addBuild(const fn_build_t& build)
{
    m_builds.push_back(build);
}

void RenderPass::addSubPass(const RenderPass* subPass)
{
    m_subPasses.push_back(subPass);
}

    }
}
