/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Context/RenderBlock.h"
#include "Render/Context/ProgramParameters.h"
#include "Render/IRenderView.h"

//#if defined(_DEBUG)
#	define T_CONTEXT_PUSH_MARKER(renderView, name) T_RENDER_PUSH_MARKER(renderView, name)
#	define T_CONTEXT_POP_MARKER(renderView) T_RENDER_POP_MARKER(renderView)
//#else
//#	define T_CONTEXT_PUSH_MARKER(renderView, name)
//#	define T_CONTEXT_POP_MARKER(renderView)
//#endif

namespace traktor::render
{

void NullRenderBlock::render(IRenderView* renderView) const
{
	if (programParams)
		programParams->fixup(program);
}

void ComputeRenderBlock::render(IRenderView* renderView) const
{
	T_CONTEXT_PUSH_MARKER(renderView, name);

	if (programParams)
		programParams->fixup(program);

	renderView->compute(
		program,
		workSize
	);

	T_CONTEXT_POP_MARKER(renderView);
}

void SimpleRenderBlock::render(IRenderView* renderView) const
{
	T_CONTEXT_PUSH_MARKER(renderView, name);

	if (programParams)
		programParams->fixup(program);

	renderView->draw(
		vertexBuffer,
		vertexLayout,
		indexBuffer,
		indexType,
		program,
		primitives,
		1
	);

	T_CONTEXT_POP_MARKER(renderView);
}

void InstancingRenderBlock::render(IRenderView* renderView) const
{
	T_CONTEXT_PUSH_MARKER(renderView, name);

	if (programParams)
		programParams->fixup(program);

	renderView->draw(
		vertexBuffer,
		vertexLayout,
		indexBuffer,
		indexType,
		program,
		primitives,
		count
	);

	T_CONTEXT_POP_MARKER(renderView);
}

void IndexedInstancingRenderBlock::render(IRenderView* renderView) const
{
	const Primitives p(primitive, offset, count, minIndex, maxIndex);

	T_CONTEXT_PUSH_MARKER(renderView, name);

	if (programParams)
		programParams->fixup(program);

	renderView->draw(
		vertexBuffer,
		vertexLayout,
		indexBuffer,
		indexType,
		program,
		p,
		instanceCount
	);

	T_CONTEXT_POP_MARKER(renderView);
}

void NonIndexedRenderBlock::render(IRenderView* renderView) const
{
	const Primitives p(primitive, offset, count);

	T_CONTEXT_PUSH_MARKER(renderView, name);

	if (programParams)
		programParams->fixup(program);

	renderView->draw(
		vertexBuffer,
		vertexLayout,
		nullptr,
		IndexType::Void,
		program,
		p,
		1
	);

	T_CONTEXT_POP_MARKER(renderView);
}

void IndexedRenderBlock::render(IRenderView* renderView) const
{
	const Primitives p(primitive, offset, count, minIndex, maxIndex);

	T_CONTEXT_PUSH_MARKER(renderView, name);

	if (programParams)
		programParams->fixup(program);

	renderView->draw(
		vertexBuffer,
		vertexLayout,
		indexBuffer,
		indexType,
		program,
		p,
		1
	);

	T_CONTEXT_POP_MARKER(renderView);
}

void IndirectRenderBlock::render(IRenderView* renderView) const
{
	T_CONTEXT_PUSH_MARKER(renderView, name);

	if (programParams)
		programParams->fixup(program);

	renderView->drawIndirect(
		vertexBuffer,
		vertexLayout,
		indexBuffer,
		indexType,
		program,
		primitive,
		drawBuffer,
		drawCount
	);

	T_CONTEXT_POP_MARKER(renderView);
}

void BeginPassRenderBlock::render(IRenderView* renderView) const
{
	T_CONTEXT_PUSH_MARKER(renderView, name);

	if (renderTargetSet)
	{
		if (renderTargetIndex >= 0)
			renderView->beginPass(renderTargetSet, renderTargetIndex, &clear, load, store);
		else
			renderView->beginPass(renderTargetSet, &clear, load, store);
	}
	else
		renderView->beginPass(&clear, load, store);
}

void EndPassRenderBlock::render(IRenderView* renderView) const
{
	renderView->endPass();

	T_CONTEXT_POP_MARKER(renderView);
}

void PresentRenderBlock::render(IRenderView* renderView) const
{
	T_CONTEXT_PUSH_MARKER(renderView, name);

	renderView->present();

	T_CONTEXT_POP_MARKER(renderView);
}

void SetViewportRenderBlock::render(IRenderView* renderView) const
{
	T_CONTEXT_PUSH_MARKER(renderView, name);

	renderView->setViewport(viewport);

	T_CONTEXT_POP_MARKER(renderView);
}

void BarrierRenderBlock::render(IRenderView* renderView) const
{
	T_CONTEXT_PUSH_MARKER(renderView, name);

	renderView->barrier(from, to, written, writtenMip);

	T_CONTEXT_POP_MARKER(renderView);
}

void LambdaRenderBlock::render(IRenderView* renderView) const
{
	T_CONTEXT_PUSH_MARKER(renderView, name);

	lambda(renderView);

	T_CONTEXT_POP_MARKER(renderView);
}

void ProfileBeginRenderBlock::render(IRenderView* renderView) const
{
	*queryHandle = renderView->beginTimeQuery();
}

void ProfileEndRenderBlock::render(IRenderView* renderView) const
{
	renderView->endTimeQuery(*queryHandle);
}

void ProfileReportRenderBlock::render(IRenderView* renderView) const
{
	double start, end;
	double offsetGPU = 0.0;

	// Get GPU offset from a reference query.
	if (referenceQueryHandle)
	{
		if (!renderView->getTimeQuery(*referenceQueryHandle, false, start, end))
			return;

		offsetGPU = start;
	}
	
	// Get GPU stamps of measured blocks.
	if (!renderView->getTimeQuery(*queryHandle, false, start, end))
		return;

	const double duration = end - start;

	start -= offsetGPU;
	start += offset;

	sink(name, start, duration);
}

}
