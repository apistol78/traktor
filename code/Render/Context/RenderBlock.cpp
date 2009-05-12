#include "Render/Context/RenderBlock.h"
#include "Render/Context/ShaderParameters.h"
#include "Render/RenderView.h"

namespace traktor
{
	namespace render
	{

void SimpleRenderBlock::render(RenderView* renderView) const
{
	if (shaderParams)
		shaderParams->fixup(shader);

	renderView->setIndexBuffer(indexBuffer);
	renderView->setVertexBuffer(vertexBuffer);
	shader->draw(renderView, *primitives);
}

void IndexedRenderBlock::render(RenderView* renderView) const
{
	Primitives p(primitive, offset, count, minIndex, maxIndex);

	if (shaderParams)
		shaderParams->fixup(shader);

	renderView->setIndexBuffer(indexBuffer);
	renderView->setVertexBuffer(vertexBuffer);
	shader->draw(renderView, p);
}

	}
}
