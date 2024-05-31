/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Matrix44.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderPass.h"
#include "Resource/IResourceManager.h"
#include "Spark/ColorTransform.h"
#include "Spark/Acc/AccQuad.h"

namespace traktor::spark
{
	namespace
	{

const resource::Id< render::Shader > c_idShaderSolid(Guid(L"{2EDC5E1B-562D-9F46-9E3C-474729FB078E}"));
const resource::Id< render::Shader > c_idShaderTextured(Guid(L"{98A59F6A-1D90-144C-B688-4CEF382453F2}"));
const resource::Id< render::Shader > c_idShaderIncrementMask(Guid(L"{16868DF6-A619-5541-83D2-94088A0AC552}"));
const resource::Id< render::Shader > c_idShaderDecrementMask(Guid(L"{D6821007-47BB-D748-9E29-20829ED09C70}"));
const resource::Id< render::Shader > c_idShaderBlit(Guid(L"{34029EA0-112D-D74B-94CA-9C32FF319BB0}"));

#pragma pack(1)
struct Vertex
{
	float pos[2];
};
#pragma pack()

const render::Handle s_handleTransform(L"Spark_Transform");
const render::Handle s_handleFrameBounds(L"Spark_FrameBounds");
const render::Handle s_handleFrameTransform(L"Spark_FrameTransform");
const render::Handle s_handleScreenOffsetScale(L"Spark_ScreenOffsetScale");
const render::Handle s_handleCxFormMul(L"Spark_CxFormMul");
const render::Handle s_handleCxFormAdd(L"Spark_CxFormAdd");
const render::Handle s_handleTexture(L"Spark_Texture");
const render::Handle s_handleTextureOffset(L"Spark_TextureOffset");

	}

bool AccQuad::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem
)
{
	if (!resourceManager->bind(c_idShaderSolid, m_shaderSolid))
		return false;
	if (!resourceManager->bind(c_idShaderTextured, m_shaderTextured))
		return false;
	if (!resourceManager->bind(c_idShaderIncrementMask, m_shaderIncrementMask))
		return false;
	if (!resourceManager->bind(c_idShaderDecrementMask, m_shaderDecrementMask))
		return false;
	if (!resourceManager->bind(c_idShaderBlit, m_shaderBlit))
		return false;

	AlignedVector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DataUsage::Position, render::DtFloat2, offsetof(Vertex, pos)));
	T_ASSERT(render::getVertexSize(vertexElements) == sizeof(Vertex));
	m_vertexLayout = renderSystem->createVertexLayout(vertexElements);

	m_vertexBuffer = renderSystem->createBuffer(render::BuVertex, 2 * 3 * sizeof(Vertex), false);
	if (!m_vertexBuffer)
		return false;

	Vertex* vertex = static_cast< Vertex* >(m_vertexBuffer->lock());
	if (!vertex)
		return false;

	vertex->pos[0] = 0.0f; vertex->pos[1] = 0.0f; ++vertex;
	vertex->pos[0] = 1.0f; vertex->pos[1] = 0.0f; ++vertex;
	vertex->pos[0] = 0.0f; vertex->pos[1] = 1.0f; ++vertex;
	vertex->pos[0] = 1.0f; vertex->pos[1] = 1.0f; ++vertex;

	m_vertexBuffer->unlock();
	return true;
}

void AccQuad::destroy()
{
	safeDestroy(m_vertexBuffer);
}

void AccQuad::render(
	render::RenderPass* renderPass,
	const Aabb2& bounds,
	const Matrix33& transform,
	const Vector4& frameBounds,
	const Vector4& frameTransform,
	const ColorTransform& cxform,
	render::ITexture* texture,
	const Vector4& textureOffset,
	bool maskWrite,
	bool maskIncrement,
	uint8_t maskReference
) const
{
	Matrix44 m1(
		transform.e11, transform.e12, transform.e13, 0.0f,
		transform.e21, transform.e22, transform.e23, 0.0f,
		transform.e31, transform.e32, transform.e33, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	Matrix44 m2(
		bounds.mx.x - bounds.mn.x, 0.0f, bounds.mn.x, 0.0f,
		0.0f, bounds.mx.y - bounds.mn.y, bounds.mn.y, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	Matrix44 m = m1 * m2;

	Ref< render::Shader > shaderSolid, shaderTextured;
	if (!maskWrite)
	{
		shaderSolid = m_shaderSolid;
		shaderTextured = m_shaderTextured;
	}
	else
	{
		if (maskIncrement)
		{
			shaderSolid = m_shaderIncrementMask;
			shaderTextured = m_shaderIncrementMask;
		}
		else
		{
			shaderSolid = m_shaderDecrementMask;
			shaderTextured = m_shaderDecrementMask;
		}
	}

	Ref< render::IProgram > program = (texture ? shaderTextured : shaderSolid)->getProgram().program;
	if (!program)
		return;

	renderPass->addBuild([=, this](const render::RenderGraph&, render::RenderContext* renderContext) {

		auto renderBlock = renderContext->allocNamed< render::NonIndexedRenderBlock >(L"Flash AccQuad");
		renderBlock->program = program;
		renderBlock->vertexBuffer = m_vertexBuffer->getBufferView();
		renderBlock->vertexLayout = m_vertexLayout;
		renderBlock->primitive = render::PrimitiveType::TriangleStrip;
		renderBlock->offset = 0;
		renderBlock->count = 2;

		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->programParams->beginParameters(renderContext);
		renderBlock->programParams->setMatrixParameter(s_handleTransform, m);
		renderBlock->programParams->setVectorParameter(s_handleFrameBounds, frameBounds);
		renderBlock->programParams->setVectorParameter(s_handleFrameTransform, frameTransform);
		renderBlock->programParams->setFloatParameter(s_handleScreenOffsetScale, 0.0f);
		renderBlock->programParams->setVectorParameter(s_handleCxFormMul, cxform.mul);
		renderBlock->programParams->setVectorParameter(s_handleCxFormAdd, cxform.add);
		renderBlock->programParams->setStencilReference(maskReference);

		if (texture)
		{
			renderBlock->programParams->setTextureParameter(s_handleTexture, texture);
			renderBlock->programParams->setVectorParameter(s_handleTextureOffset, textureOffset);
		}

		renderBlock->programParams->endParameters(renderContext);

		renderContext->draw(renderBlock);

	});
}

void AccQuad::blit(
	render::RenderPass* renderPass,
	render::ITexture* texture
) const
{
	renderPass->addBuild([=, this](const render::RenderGraph&, render::RenderContext* renderContext) {

		auto renderBlock = renderContext->allocNamed< render::NonIndexedRenderBlock >(L"Flash AccQuad (blit)");
		renderBlock->program = m_shaderBlit->getProgram().program;
		renderBlock->vertexBuffer = m_vertexBuffer->getBufferView();
		renderBlock->vertexLayout = m_vertexLayout;
		renderBlock->primitive = render::PrimitiveType::TriangleStrip;
		renderBlock->offset = 0;
		renderBlock->count = 2;

		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->programParams->beginParameters(renderContext);
		renderBlock->programParams->setTextureParameter(s_handleTexture, texture);
		renderBlock->programParams->endParameters(renderContext);

		renderContext->draw(renderBlock);

	});
}

}
