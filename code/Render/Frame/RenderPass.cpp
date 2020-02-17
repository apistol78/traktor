#include "Render/Frame/RenderPass.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderPass", RenderPass, Object)

RenderPass::RenderPass(const std::wstring& name)
:   m_name(name)
{
}

void RenderPass::addInput(handle_t targetSetId)
{
	// Just ignore invalid target set id; convenient when setting up passes.
	if (targetSetId == 0)
		return;

	auto& input = m_inputs.push_back();
	input.targetSetId = targetSetId;
	input.colorIndex = 0;
}

AlignedVector< RenderPass::Input > RenderPass::getInputs() const
{
	AlignedVector< Input > inputs = m_inputs;
	for (auto subPass : m_subPasses)
		inputs.insert(inputs.end(), subPass->m_inputs.begin(), subPass->m_inputs.end());
	return inputs;
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

void RenderPass::addSubPass(const RenderPass* subPass)
{
	m_subPasses.push_back(subPass);
}

	}
}
