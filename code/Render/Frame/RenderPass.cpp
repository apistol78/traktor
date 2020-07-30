#include "Render/Frame/RenderPass.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderPass", RenderPass, Object)

RenderPass::RenderPass(const std::wstring& name)
:   m_name(name)
{
	m_output.targetSetId = ~0;
}

void RenderPass::setName(const std::wstring& name)
{
	m_name = name;
}

void RenderPass::addInput(handle_t targetSetId, bool useDepth)
{
	// Just ignore invalid target set id; convenient when setting up passes to be able to add null inputs.
	if (targetSetId == 0)
		return;

	auto& input = m_inputs.push_back();
	input.targetSetId = targetSetId;
	input.useDepth = useDepth;
}

void RenderPass::setOutput(handle_t targetSetId)
{
	m_output.targetSetId = targetSetId;
	m_output.clear.mask = 0;
}

void RenderPass::setOutput(handle_t targetSetId, const Clear& clear)
{
	m_output.targetSetId = targetSetId;
	m_output.clear = clear;
}

void RenderPass::addBuild(const fn_build_t& build)
{
	m_builds.push_back(build);
}

	}
}
