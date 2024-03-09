/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Math/Aabb2.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/Shader.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Frame/RenderPass.h"
#include "Resource/IResourceManager.h"
#include "Spark/Acc/AccGlyph.h"

namespace traktor::spark
{
	namespace
	{

#pragma pack(1)
struct Vertex
{
	float pos[2];
	float texCoord[2];
	float texOffsetAndScale[4];
};
#pragma pack()

const resource::Id< render::Shader > c_idShaderGlyphMask(Guid(L"{C8FEF24B-D775-A14D-9FF3-E34A17495FB4}"));
const uint32_t c_glyphCount = 1000;

const struct TemplateVertex
{
	Vector4 pos;
	Vector2 texCoord;
}
c_glyphTemplate[4] =
{
	{ Vector4(-0.1f, -0.1f, 1.0f, 0.0f), Vector2(-0.1f, -0.1f) },
	{ Vector4( 1.1f, -0.1f, 1.0f, 0.0f), Vector2( 1.1f, -0.1f) },
	{ Vector4( 1.1f,  1.1f, 1.0f, 0.0f), Vector2( 1.1f,  1.1f) },
	{ Vector4(-0.1f,  1.1f, 1.0f, 0.0f), Vector2(-0.1f,  1.1f) }
};

const render::Handle s_handleTechniqueDefault(L"Default");
const render::Handle s_handleTechniqueDropShadow(L"DropShadow");
const render::Handle s_handleTechniqueGlow(L"Glow");
const render::Handle s_handleFrameBounds(L"Spark_FrameBounds");
const render::Handle s_handleFrameTransform(L"Spark_FrameTransform");
const render::Handle s_handleTexture(L"Spark_Texture");
const render::Handle s_handleColor(L"Spark_Color");
const render::Handle s_handleFilterColor(L"Spark_FilterColor");

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.AccGlyph", AccGlyph, Object)

bool AccGlyph::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem
)
{
	if (!resourceManager->bind(c_idShaderGlyphMask, m_shaderGlyph))
		return false;

	AlignedVector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DataUsage::Position, render::DtFloat2, offsetof(Vertex, pos)));
	vertexElements.push_back(render::VertexElement(render::DataUsage::Custom, render::DtFloat2, offsetof(Vertex, texCoord)));
	vertexElements.push_back(render::VertexElement(render::DataUsage::Custom, render::DtFloat4, offsetof(Vertex, texOffsetAndScale), 1));
	T_ASSERT(render::getVertexSize(vertexElements) == sizeof(Vertex));
	m_vertexLayout = renderSystem->createVertexLayout(vertexElements);

	m_vertexBuffer = renderSystem->createBuffer(
		render::BuVertex,
		c_glyphCount * sizeof_array(c_glyphTemplate) * sizeof(Vertex),
		true
	);
	if (!m_vertexBuffer)
		return false;

	m_indexBuffer = renderSystem->createBuffer(render::BuIndex, c_glyphCount * 6 * sizeof(uint16_t), false);
	if (!m_indexBuffer)
		return false;

	uint16_t* index = (uint16_t*)m_indexBuffer->lock();
	if (!index)
		return false;

	for (uint32_t i = 0; i < c_glyphCount; ++i)
	{
		uint16_t base = i * 4;
		*index++ = base + 0;
		*index++ = base + 1;
		*index++ = base + 2;
		*index++ = base + 0;
		*index++ = base + 2;
		*index++ = base + 3;
	}

	m_indexBuffer->unlock();
	return true;
}

void AccGlyph::destroy()
{
	safeDestroy(m_indexBuffer);
	safeDestroy(m_vertexBuffer);
}

void AccGlyph::beginFrame()
{
	m_vertex = (uint8_t*)m_vertexBuffer->lock();
	m_offset = 0;
	m_count = 0;
}

void AccGlyph::endFrame()
{
	T_FATAL_ASSERT (m_count == 0);
	m_vertexBuffer->unlock();
}

void AccGlyph::add(
	const Aabb2& bounds,
	const Matrix33& transform,
	const Vector4& textureOffset
)
{
	T_FATAL_ASSERT (m_vertex != nullptr);

	if (m_count + m_offset >= c_glyphCount)
	{
		log::warning << L"Too many glyphs cached; skipped" << Endl;
		return;
	}

	float dbx = bounds.mx.x - bounds.mn.x;
	float dby = bounds.mx.y - bounds.mn.y;

	Matrix44 m1(
		transform.e11, transform.e12, transform.e13, 0.0f,
		transform.e21, transform.e22, transform.e23, 0.0f,
		transform.e31, transform.e32, transform.e33, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	Matrix44 m2(
		dbx, 0.0f, bounds.mn.x, 0.0f,
		0.0f, dby, bounds.mn.y, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	Matrix44 m = m1 * m2;

	Vertex* vertex = reinterpret_cast< Vertex* >(m_vertex);
	for (uint32_t i = 0; i < sizeof_array(c_glyphTemplate); ++i)
	{
		Vector4 pos = m * c_glyphTemplate[i].pos;

		vertex->pos[0] = pos.x();
		vertex->pos[1] = pos.y();
		vertex->texCoord[0] = c_glyphTemplate[i].texCoord.x;
		vertex->texCoord[1] = c_glyphTemplate[i].texCoord.y;

		vertex->texOffsetAndScale[0] = textureOffset.x();
		vertex->texOffsetAndScale[1] = textureOffset.y();

		vertex->texOffsetAndScale[2] = dbx;
		vertex->texOffsetAndScale[3] = dby;

		vertex++;
	}

	m_vertex += sizeof(Vertex) * sizeof_array(c_glyphTemplate);
	m_count++;
}

void AccGlyph::render(
	render::RenderPass* renderPass,
	render::handle_t glyphCacheTargetSetId,
	const Vector4& frameBounds,
	const Vector4& frameTransform,
	uint8_t maskReference,
	uint8_t glyphFilter,
	const Color4f& glyphColor,
	const Color4f& glyphFilterColor
)
{
	if (!m_count)
		return;

	const render::handle_t techniques[] = { s_handleTechniqueDefault, s_handleTechniqueDropShadow, 0, s_handleTechniqueGlow };
	T_ASSERT(glyphFilter < sizeof_array(techniques));

	const render::Shader::Permutation perm(techniques[glyphFilter]);

	auto sp = m_shaderGlyph->getProgram(perm);
	if (!sp)
		return;

	uint32_t offset = m_offset;
	uint32_t count = m_count;

	renderPass->addBuild([=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext) {

		auto glyphCacheTargetSet = renderGraph.getTargetSet(glyphCacheTargetSetId);

		render::IndexedRenderBlock* renderBlock = renderContext->allocNamed< render::IndexedRenderBlock >(L"Flash AccGlyph");
		renderBlock->program = sp.program;
		renderBlock->indexBuffer = m_indexBuffer->getBufferView();
		renderBlock->indexType = render::IndexType::UInt16;
		renderBlock->vertexBuffer = m_vertexBuffer->getBufferView();
		renderBlock->vertexLayout = m_vertexLayout;
		renderBlock->primitive = render::PrimitiveType::Triangles;
		renderBlock->offset = offset * 6;
		renderBlock->count = count * 2;
		renderBlock->minIndex = 0;
		renderBlock->maxIndex = c_glyphCount * 4 - 1;

		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->programParams->beginParameters(renderContext);
		renderBlock->programParams->setVectorParameter(s_handleFrameBounds, frameBounds);
		renderBlock->programParams->setVectorParameter(s_handleFrameTransform, frameTransform);
		renderBlock->programParams->setStencilReference(maskReference);
		renderBlock->programParams->setTextureParameter(s_handleTexture, glyphCacheTargetSet->getColorTexture(0)); // texture);
		renderBlock->programParams->setVectorParameter(s_handleColor, glyphColor);

		if (glyphFilter != 0)
			renderBlock->programParams->setVectorParameter(s_handleFilterColor, glyphFilterColor);

		renderBlock->programParams->endParameters(renderContext);

		renderContext->draw(renderBlock);

	});

	m_offset += m_count;
	m_count = 0;
}

}
