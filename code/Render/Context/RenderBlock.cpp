/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Context/RenderBlock.h"
#include "Render/Context/ProgramParameters.h"
#include "Render/IRenderView.h"

#if defined(_DEBUG)
#	define T_CONTEXT_PUSH_MARKER(renderView, name) T_RENDER_PUSH_MARKER(renderView, name)
#	define T_CONTEXT_POP_MARKER(renderView) T_RENDER_POP_MARKER(renderView)
#else
#	define T_CONTEXT_PUSH_MARKER(renderView, name)
#	define T_CONTEXT_POP_MARKER(renderView)
#endif

namespace traktor
{
	namespace render
	{

void NullRenderBlock::render(IRenderView* renderView, const ProgramParameters* globalParameters) const
{
	T_CONTEXT_PUSH_MARKER(renderView, name);

	if (globalParameters)
		globalParameters->fixup(program);
	if (programParams)
		programParams->fixup(program);

	T_CONTEXT_POP_MARKER(renderView);
}

void SimpleRenderBlock::render(IRenderView* renderView, const ProgramParameters* globalParameters) const
{
	T_CONTEXT_PUSH_MARKER(renderView, name);

	if (globalParameters)
		globalParameters->fixup(program);
	if (programParams)
		programParams->fixup(program);

	renderView->draw(
		vertexBuffer,
		indexBuffer,
		program,
		primitives
	);

	T_CONTEXT_POP_MARKER(renderView);
}

void InstancingRenderBlock::render(IRenderView* renderView, const ProgramParameters* globalParameters) const
{
	T_CONTEXT_PUSH_MARKER(renderView, name);

	if (globalParameters)
		globalParameters->fixup(program);
	if (programParams)
		programParams->fixup(program);

	renderView->draw(
		vertexBuffer,
		indexBuffer,
		program,
		primitives,
		count
	);

	T_CONTEXT_POP_MARKER(renderView);
}

void IndexedInstancingRenderBlock::render(IRenderView* renderView, const ProgramParameters* globalParameters) const
{
	Primitives p(primitive, offset, count, minIndex, maxIndex);

	T_CONTEXT_PUSH_MARKER(renderView, name);

	if (globalParameters)
		globalParameters->fixup(program);
	if (programParams)
		programParams->fixup(program);

	renderView->draw(
		vertexBuffer,
		indexBuffer,
		program,
		p,
		instanceCount
	);

	T_CONTEXT_POP_MARKER(renderView);
}

void NonIndexedRenderBlock::render(IRenderView* renderView, const ProgramParameters* globalParameters) const
{
	Primitives p(primitive, offset, count);

	T_CONTEXT_PUSH_MARKER(renderView, name);

	if (globalParameters)
		globalParameters->fixup(program);
	if (programParams)
		programParams->fixup(program);

	renderView->draw(
		vertexBuffer,
		0,
		program,
		p
	);

	T_CONTEXT_POP_MARKER(renderView);
}

void IndexedRenderBlock::render(IRenderView* renderView, const ProgramParameters* globalParameters) const
{
	Primitives p(primitive, offset, count, minIndex, maxIndex);

	T_CONTEXT_PUSH_MARKER(renderView, name);

	if (globalParameters)
		globalParameters->fixup(program);
	if (programParams)
		programParams->fixup(program);

	renderView->draw(
		vertexBuffer,
		indexBuffer,
		program,
		p
	);

	T_CONTEXT_POP_MARKER(renderView);
}

void TargetBeginRenderBlock::render(IRenderView* renderView, const ProgramParameters* globalParameters) const
{
	T_CONTEXT_PUSH_MARKER(renderView, name);

	renderView->begin(renderTargetSet, renderTargetIndex);

	T_CONTEXT_POP_MARKER(renderView);
}

void TargetEndRenderBlock::render(IRenderView* renderView, const ProgramParameters* globalParameters) const
{
	T_CONTEXT_PUSH_MARKER(renderView, name);

	renderView->end();

	T_CONTEXT_POP_MARKER(renderView);
}

void TargetClearRenderBlock::render(IRenderView* renderView, const ProgramParameters* globalParameters) const
{
	T_CONTEXT_PUSH_MARKER(renderView, name);

	if (clearMask)
	{
		renderView->clear(
			clearMask,
			&clearColor,
			clearDepth,
			clearStencil
		);
	}

	T_CONTEXT_POP_MARKER(renderView);
}

void ChainRenderBlock::render(IRenderView* renderView, const ProgramParameters* globalParameters) const
{
	T_CONTEXT_PUSH_MARKER(renderView, name);

	if (inner)
		inner->render(renderView, globalParameters);

	if (next)
		next->render(renderView, globalParameters);

	T_CONTEXT_POP_MARKER(renderView);
}

	}
}
