#include "Render/Frame/RenderPass.h"
#include "Render/Frame/RenderPassBuilder.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderPassBuilder", RenderPassBuilder, Object)

RenderPassBuilder::RenderPassBuilder(RenderPass& renderPass)
:   m_renderPass(renderPass)
{
}

void RenderPassBuilder::addInput(handle_t targetSetName, int32_t targetColorIndex)
{
	auto& input = m_renderPass.m_inputs.push_back();
	input.targetSetName = targetSetName;
	input.targetColorIndex = targetColorIndex;
}

void RenderPassBuilder::setOutput(handle_t targetSetName)
{
	auto& output = m_renderPass.m_output;
	output.targetSetName = targetSetName;
	output.targetColorIndex = -1;
	output.clear.mask = 0;
}

void RenderPassBuilder::setOutput(handle_t targetSetName, const Clear& clear)
{
	auto& output = m_renderPass.m_output;
	output.targetSetName = targetSetName;
	output.targetColorIndex = -1;
	output.clear = clear;
}

	}
}