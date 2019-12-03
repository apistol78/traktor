#include "Core/Misc/SafeDestroy.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/IRenderView.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

#pragma pack(1)
struct ScreenVertex
{
	float pos[2];
	float texCoord[2];
};
#pragma pack()

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ScreenRenderer", ScreenRenderer, Object)

ScreenRenderer::ScreenRenderer()
{
}

bool ScreenRenderer::create(IRenderSystem* renderSystem)
{
	AlignedVector< VertexElement > vertexElements;
	vertexElements.push_back(VertexElement(DuPosition, DtFloat2, offsetof(ScreenVertex, pos)));
	vertexElements.push_back(VertexElement(DuCustom, DtFloat2, offsetof(ScreenVertex, texCoord)));
	T_FATAL_ASSERT(getVertexSize(vertexElements) == sizeof(ScreenVertex));

	m_vertexBuffer = renderSystem->createVertexBuffer(vertexElements, 6 * sizeof(ScreenVertex), false);
	if (!m_vertexBuffer)
		return false;

	ScreenVertex* vertex = reinterpret_cast< ScreenVertex* >(m_vertexBuffer->lock());
	T_ASSERT(vertex);

	vertex[2].pos[0] = -1.0f; vertex[2].pos[1] =  1.0f; vertex[2].texCoord[0] = 0.0f; vertex[2].texCoord[1] = 0.0f;
	vertex[1].pos[0] =  1.0f; vertex[1].pos[1] =  1.0f; vertex[1].texCoord[0] = 1.0f; vertex[1].texCoord[1] = 0.0f;
	vertex[0].pos[0] =  1.0f; vertex[0].pos[1] = -1.0f; vertex[0].texCoord[0] = 1.0f; vertex[0].texCoord[1] = 1.0f;

	vertex[5].pos[0] = -1.0f; vertex[5].pos[1] =  1.0f; vertex[5].texCoord[0] = 0.0f; vertex[5].texCoord[1] = 0.0f;
	vertex[4].pos[0] =  1.0f; vertex[4].pos[1] = -1.0f; vertex[4].texCoord[0] = 1.0f; vertex[4].texCoord[1] = 1.0f;
	vertex[3].pos[0] = -1.0f; vertex[3].pos[1] = -1.0f; vertex[3].texCoord[0] = 0.0f; vertex[3].texCoord[1] = 1.0f;

	m_vertexBuffer->unlock();

	m_primitives.setNonIndexed(PtTriangles, 0, 2);
	return true;
}

void ScreenRenderer::destroy()
{
	safeDestroy(m_vertexBuffer);
}

void ScreenRenderer::draw(IRenderView* renderView, IProgram* program)
{
	renderView->draw(m_vertexBuffer, nullptr, program, m_primitives);
}

void ScreenRenderer::draw(IRenderView* renderView, Shader* shader)
{
	shader->draw(renderView, m_vertexBuffer, nullptr, m_primitives);
}

void ScreenRenderer::draw(IRenderView* renderView, IRenderTargetSet* renderTargetSet, int32_t renderTarget, IProgram* program)
{
	if (renderView->begin(renderTargetSet, renderTarget, nullptr))
	{
		renderView->draw(m_vertexBuffer, nullptr, program, m_primitives);
		renderView->end();
	}
}

void ScreenRenderer::draw(IRenderView* renderView, IRenderTargetSet* renderTargetSet, int32_t renderTarget, Shader* shader)
{
	if (renderView->begin(renderTargetSet, renderTarget, nullptr))
	{
		shader->draw(renderView, m_vertexBuffer, nullptr, m_primitives);
		renderView->end();
	}
}

	}
}
