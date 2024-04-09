/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/IVertexLayout.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"

namespace traktor::render
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

bool ScreenRenderer::create(IRenderSystem* renderSystem)
{
	AlignedVector< VertexElement > vertexElements;
	vertexElements.push_back(VertexElement(DataUsage::Position, DtFloat2, offsetof(ScreenVertex, pos)));
	vertexElements.push_back(VertexElement(DataUsage::Custom, DtFloat2, offsetof(ScreenVertex, texCoord)));
	m_vertexLayout = renderSystem->createVertexLayout(vertexElements);
	if (!m_vertexLayout)
		return false;

	m_vertexBuffer = renderSystem->createBuffer(BuVertex, 6 * sizeof(ScreenVertex), false);
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

	m_primitives.setNonIndexed(PrimitiveType::Triangles, 0, 2);
	return true;
}

void ScreenRenderer::destroy()
{
	safeDestroy(m_vertexBuffer);
}

void ScreenRenderer::draw(IRenderView* renderView, IProgram* program)
{
	if (program)
		renderView->draw(m_vertexBuffer->getBufferView(), m_vertexLayout, nullptr, IndexType::Void, program, m_primitives, 1);
}

void ScreenRenderer::draw(IRenderView* renderView, const Shader* shader)
{
	IProgram* program = shader->getProgram().program;
	if (program)
		draw(renderView, program);
}

void ScreenRenderer::draw(IRenderView* renderView, const Shader* shader, const Shader::Permutation& permutation)
{
	if (!shader)
		return;

	IProgram* program = shader->getProgram(permutation).program;
	draw(renderView, program);
}

void ScreenRenderer::draw(RenderContext* renderContext, IProgram* program, ProgramParameters* programParams, const wchar_t* name)
{
	if (!program)
		return;

	auto rb = renderContext->allocNamed< SimpleRenderBlock >(name != nullptr ? name : L"Screen renderer");
	rb->program = program;
	rb->programParams = programParams;
	rb->indexBuffer = nullptr;
	rb->vertexBuffer = m_vertexBuffer->getBufferView();
	rb->vertexLayout = m_vertexLayout;
	rb->primitives = m_primitives;
	renderContext->draw(rb);
}

void ScreenRenderer::draw(RenderContext* renderContext, const Shader* shader, ProgramParameters* programParams, const wchar_t* name)
{
	if (!shader)
		return;

	auto program = shader->getProgram();
	if (!program)
		return;

	auto rb = renderContext->allocNamed< SimpleRenderBlock >(name != nullptr ? name : L"Screen renderer");
	rb->program = program.program;
	rb->programParams = programParams;
	rb->indexBuffer = nullptr;
	rb->vertexBuffer = m_vertexBuffer->getBufferView();
	rb->vertexLayout = m_vertexLayout;
	rb->primitives = m_primitives;
	renderContext->draw(program.priority, rb);
}

void ScreenRenderer::draw(RenderContext* renderContext, const Shader* shader, const Shader::Permutation& permutation, ProgramParameters* programParams, const wchar_t* name)
{
	if (!shader)
		return;

	auto program = shader->getProgram(permutation);
	if (!program)
		return;

	auto rb = renderContext->allocNamed< SimpleRenderBlock >(name != nullptr ? name : L"Screen renderer");
	rb->program = program.program;
	rb->programParams = programParams;
	rb->indexBuffer = nullptr;
	rb->vertexBuffer = m_vertexBuffer->getBufferView();
	rb->vertexLayout = m_vertexLayout;
	rb->primitives = m_primitives;
	renderContext->draw(program.priority, rb);
}

}
