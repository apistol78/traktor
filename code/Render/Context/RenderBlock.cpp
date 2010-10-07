#include "Render/Context/RenderBlock.h"
#include "Render/Context/ProgramParameters.h"
#include "Render/IRenderView.h"

namespace traktor
{
	namespace render
	{

void NullRenderBlock::render(IRenderView* renderView, const ProgramParameters* globalParameters) const
{
	T_RENDER_PUSH_MARKER(renderView, name);

	if (programParams)
		programParams->fixup(program);
	if (globalParameters)
		globalParameters->fixup(program);

	T_RENDER_POP_MARKER(renderView);
}

void SimpleRenderBlock::render(IRenderView* renderView, const ProgramParameters* globalParameters) const
{
	T_RENDER_PUSH_MARKER(renderView, name);

	if (programParams)
		programParams->fixup(program);
	if (globalParameters)
		globalParameters->fixup(program);

	renderView->setIndexBuffer(indexBuffer);
	renderView->setVertexBuffer(vertexBuffer);
	renderView->setProgram(program);
	renderView->draw(*primitives);

	T_RENDER_POP_MARKER(renderView);
}

void NonIndexedRenderBlock::render(IRenderView* renderView, const ProgramParameters* globalParameters) const
{
	Primitives p(primitive, offset, count);

	T_RENDER_PUSH_MARKER(renderView, name);

	if (programParams)
		programParams->fixup(program);
	if (globalParameters)
		globalParameters->fixup(program);

	renderView->setVertexBuffer(vertexBuffer);
	renderView->setProgram(program);
	renderView->draw(p);

	T_RENDER_POP_MARKER(renderView);
}

void IndexedRenderBlock::render(IRenderView* renderView, const ProgramParameters* globalParameters) const
{
	Primitives p(primitive, offset, count, minIndex, maxIndex);

	T_RENDER_PUSH_MARKER(renderView, name);

	if (programParams)
		programParams->fixup(program);
	if (globalParameters)
		globalParameters->fixup(program);

	renderView->setIndexBuffer(indexBuffer);
	renderView->setVertexBuffer(vertexBuffer);
	renderView->setProgram(program);
	renderView->draw(p);

	T_RENDER_POP_MARKER(renderView);
}

void TargetBeginRenderBlock::render(IRenderView* renderView, const ProgramParameters* globalParameters) const
{
	T_RENDER_PUSH_MARKER(renderView, name);

	renderView->begin(renderTargetSet, renderTargetIndex, keepDepthStencil);

	T_RENDER_POP_MARKER(renderView);
}

void TargetEndRenderBlock::render(IRenderView* renderView, const ProgramParameters* globalParameters) const
{
	T_RENDER_PUSH_MARKER(renderView, name);

	renderView->end();

	T_RENDER_POP_MARKER(renderView);
}

void TargetClearRenderBlock::render(IRenderView* renderView, const ProgramParameters* globalParameters) const
{
	T_RENDER_PUSH_MARKER(renderView, name);

	if (clearMask)
	{
		renderView->clear(
			clearMask,
			clearColor,
			clearDepth,
			clearStencil
		);
	}

	T_RENDER_POP_MARKER(renderView);
}

void ChainRenderBlock::render(IRenderView* renderView, const ProgramParameters* globalParameters) const
{
	T_RENDER_PUSH_MARKER(renderView, name);

	if (inner)
		inner->render(renderView, globalParameters);

	if (next)
		next->render(renderView, globalParameters);

	T_RENDER_POP_MARKER(renderView);
}

	}
}
