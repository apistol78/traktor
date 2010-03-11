#include "Render/Context/RenderBlock.h"
#include "Render/Context/ShaderParameters.h"
#include "Render/IRenderView.h"

namespace traktor
{
	namespace render
	{

void NullRenderBlock::render(IRenderView* renderView) const
{
#if defined(T_USE_RENDERBLOCK_NAME)
	if (name)
		renderView->pushMarker(name);
#endif

	if (shaderParams)
		shaderParams->fixup(shader);

#if defined(T_USE_RENDERBLOCK_NAME)
	if (name)
		renderView->popMarker();
#endif
}

void SimpleRenderBlock::render(IRenderView* renderView) const
{
#if defined(T_USE_RENDERBLOCK_NAME)
	if (name)
		renderView->pushMarker(name);
#endif

	if (shaderParams)
		shaderParams->fixup(shader);

	renderView->setIndexBuffer(indexBuffer);
	renderView->setVertexBuffer(vertexBuffer);
	shader->draw(renderView, *primitives);

#if defined(T_USE_RENDERBLOCK_NAME)
	if (name)
		renderView->popMarker();
#endif
}

void NonIndexedRenderBlock::render(IRenderView* renderView) const
{
	Primitives p(primitive, offset, count);

#if defined(T_USE_RENDERBLOCK_NAME)
	if (name)
		renderView->pushMarker(name);
#endif

	if (shaderParams)
		shaderParams->fixup(shader);

	renderView->setVertexBuffer(vertexBuffer);
	shader->draw(renderView, p);

#if defined(T_USE_RENDERBLOCK_NAME)
	if (name)
		renderView->popMarker();
#endif
}

void IndexedRenderBlock::render(IRenderView* renderView) const
{
	Primitives p(primitive, offset, count, minIndex, maxIndex);

#if defined(T_USE_RENDERBLOCK_NAME)
	if (name)
		renderView->pushMarker(name);
#endif

	if (shaderParams)
		shaderParams->fixup(shader);

	renderView->setIndexBuffer(indexBuffer);
	renderView->setVertexBuffer(vertexBuffer);
	shader->draw(renderView, p);

#if defined(T_USE_RENDERBLOCK_NAME)
	if (name)
		renderView->popMarker();
#endif
}

void TargetBeginRenderBlock::render(IRenderView* renderView) const
{
#if defined(T_USE_RENDERBLOCK_NAME)
	if (name)
		renderView->pushMarker(name);
#endif

	renderView->begin(renderTargetSet, renderTargetIndex, keepDepthStencil);

#if defined(T_USE_RENDERBLOCK_NAME)
	if (name)
		renderView->popMarker();
#endif
}

void TargetEndRenderBlock::render(IRenderView* renderView) const
{
#if defined(T_USE_RENDERBLOCK_NAME)
	if (name)
		renderView->pushMarker(name);
#endif

	renderView->end();

#if defined(T_USE_RENDERBLOCK_NAME)
	if (name)
		renderView->popMarker();
#endif
}

void TargetClearRenderBlock::render(IRenderView* renderView) const
{
#if defined(T_USE_RENDERBLOCK_NAME)
	if (name)
		renderView->pushMarker(name);
#endif

	if (clearMask)
	{
		renderView->clear(
			clearMask,
			clearColor,
			clearDepth,
			clearStencil
		);
	}

#if defined(T_USE_RENDERBLOCK_NAME)
	if (name)
		renderView->popMarker();
#endif
}

void ChainRenderBlock::render(IRenderView* renderView) const
{
#if defined(T_USE_RENDERBLOCK_NAME)
	if (name)
		renderView->pushMarker(name);
#endif

	if (inner)
		inner->render(renderView);

	if (next)
		next->render(renderView);

#if defined(T_USE_RENDERBLOCK_NAME)
	if (name)
		renderView->popMarker();
#endif
}

	}
}
