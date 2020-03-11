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

StaticVector< RenderPass::Input, 16 > RenderPass::getInputs() const
{
	auto inputs = m_inputs;
	for (auto subPass : m_subPasses)
	{
		for (const auto& subInput : subPass->m_inputs)
			inputs.push_back(subInput);
	}
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
