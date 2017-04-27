/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/IRenderView.h"
#include "Render/IRenderSystem.h"
#include "Render/RenderTargetSet.h"
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
	std::vector< VertexElement > vertexElements;
	vertexElements.push_back(VertexElement(DuPosition, DtFloat2, offsetof(ScreenVertex, pos)));
	vertexElements.push_back(VertexElement(DuCustom, DtFloat2, offsetof(ScreenVertex, texCoord)));

	m_vertexBuffer = renderSystem->createVertexBuffer(vertexElements, 6 * sizeof(ScreenVertex), false);
	if (!m_vertexBuffer)
		return false;

	ScreenVertex* vertex = reinterpret_cast< ScreenVertex* >(m_vertexBuffer->lock());
	T_ASSERT (vertex);

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
	if (m_vertexBuffer)
	{
		m_vertexBuffer->destroy();
		m_vertexBuffer = 0;
	}
}

void ScreenRenderer::draw(IRenderView* renderView, IProgram* program)
{
	renderView->draw(m_vertexBuffer, 0, program, m_primitives);
}

void ScreenRenderer::draw(IRenderView* renderView, Shader* shader)
{
	shader->draw(renderView, m_vertexBuffer, 0, m_primitives);
}

void ScreenRenderer::draw(IRenderView* renderView, RenderTargetSet* renderTargetSet, int renderTarget, IProgram* program)
{
	if (renderView->begin(renderTargetSet, renderTarget))
	{
		renderView->draw(m_vertexBuffer, 0, program, m_primitives);
		renderView->end();
	}
}

void ScreenRenderer::draw(IRenderView* renderView, RenderTargetSet* renderTargetSet, int renderTarget, Shader* shader)
{
	if (renderView->begin(renderTargetSet, renderTarget))
	{
		shader->draw(renderView, m_vertexBuffer, 0, m_primitives);
		renderView->end();
	}
}

	}
}
