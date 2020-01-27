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

void RenderPassBuilder::addInput(handle_t name, int32_t colorIndex)
{
	auto& input = m_renderPass.m_inputs.push_back();
	input.name = name;
	input.colorIndex = colorIndex;
	input.history = false;
}

void RenderPassBuilder::addHistoryInput(handle_t name, int32_t colorIndex)
{
	auto& input = m_renderPass.m_inputs.push_back();
	input.name = name;
	input.colorIndex = colorIndex;
	input.history = true;
}

void RenderPassBuilder::setOutput(handle_t name)
{
	auto& output = m_renderPass.m_output;
	output.name = name;
	output.colorIndex = -1;
	output.clear.mask = 0;
}

void RenderPassBuilder::setOutput(handle_t name, const Clear& clear)
{
	auto& output = m_renderPass.m_output;
	output.name = name;
	output.colorIndex = -1;
	output.clear = clear;
}

	}
}