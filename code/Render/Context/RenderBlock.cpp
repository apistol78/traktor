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

void IndexedRenderBlock::render(IRenderView* renderView) const
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
