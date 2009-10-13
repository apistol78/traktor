#include "Render/Context/RenderBlock.h"
#include "Render/Context/ShaderParameters.h"
#include "Render/IRenderView.h"

namespace traktor
{
	namespace render
	{

void SimpleRenderBlock::render(IRenderView* renderView) const
{
	if (shaderParams)
		shaderParams->fixup(shader);

	renderView->setIndexBuffer(indexBuffer);
	renderView->setVertexBuffer(vertexBuffer);
	shader->draw(renderView, *primitives);
}

void NonIndexedRenderBlock::render(IRenderView* renderView) const
{
	Primitives p(primitive, offset, count);

	if (shaderParams)
		shaderParams->fixup(shader);

	renderView->setVertexBuffer(vertexBuffer);
	shader->draw(renderView, p);
}

void IndexedRenderBlock::render(IRenderView* renderView) const
{
	Primitives p(primitive, offset, count, minIndex, maxIndex);

	if (shaderParams)
		shaderParams->fixup(shader);

	renderView->setIndexBuffer(indexBuffer);
	renderView->setVertexBuffer(vertexBuffer);
	shader->draw(renderView, p);
}

void TargetRenderBlock::render(IRenderView* renderView) const
{
	if (renderView->begin(renderTargetSet, renderTargetIndex, keepDepthStencil))
	{
		if (clearMask)
			renderView->clear(clearMask, clearColor, clearDepth, clearStencil);

		if (inner)
			inner->render(renderView);

		renderView->end();
	}
}

void ChainRenderBlock::render(IRenderView* renderView) const
{
	if (inner)
		inner->render(renderView);

	if (next)
		next->render(renderView);
}

	}
}
