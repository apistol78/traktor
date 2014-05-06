#include "Core/Log/Log.h"
#include "Core/Math/Aabb2.h"
#include "Core/Misc/SafeDestroy.h"
#include "Flash/SwfTypes.h"
#include "Flash/Acc/AccGlyph.h"
#include "Render/IndexBuffer.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace flash
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

const resource::Id< render::Shader > c_idShaderGlyph(Guid(L"{A8BC2D03-EB52-B744-8D4B-29E39FF0B4F5}"));
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

bool s_handleInitialized = false;
render::handle_t s_handleFrameSize;
render::handle_t s_handleViewSize;
render::handle_t s_handleViewOffset;
render::handle_t s_handleScreenOffsetScale;
render::handle_t s_handleTexture;
render::handle_t s_handleColor;
render::handle_t s_handleFilterColor;
render::handle_t s_handleTechniqueDefault;
render::handle_t s_handleTechniqueDropShadow;
render::handle_t s_handleTechniqueGlow;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AccGlyph", AccGlyph, Object)

AccGlyph::AccGlyph()
:	m_currentVertexBuffer(0)
,	m_vertex(0)
,	m_count(0)
{
}

bool AccGlyph::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem
)
{
	if (!s_handleInitialized)
	{
		s_handleFrameSize = render::getParameterHandle(L"Flash_FrameSize");
		s_handleViewSize = render::getParameterHandle(L"Flash_ViewSize");
		s_handleViewOffset = render::getParameterHandle(L"Flash_ViewOffset");
		s_handleScreenOffsetScale = render::getParameterHandle(L"Flash_ScreenOffsetScale");
		s_handleTexture = render::getParameterHandle(L"Flash_Texture");
		s_handleColor = render::getParameterHandle(L"Flash_Color");
		s_handleFilterColor = render::getParameterHandle(L"Flash_FilterColor");
		s_handleTechniqueDefault = render::getParameterHandle(L"Default");
		s_handleTechniqueDropShadow = render::getParameterHandle(L"DropShadow");
		s_handleTechniqueGlow = render::getParameterHandle(L"Glow");
		s_handleInitialized = true;
	}

	if (!resourceManager->bind(c_idShaderGlyph, m_shaderGlyph))
		return false;

	if (!resourceManager->bind(c_idShaderGlyphMask, m_shaderGlyphMask))
		return false;

	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat2, offsetof(Vertex, pos)));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat2, offsetof(Vertex, texCoord)));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat4, offsetof(Vertex, texOffsetAndScale), 1));
	T_ASSERT (render::getVertexSize(vertexElements) == sizeof(Vertex));

	for (uint32_t i = 0; i < sizeof_array(m_vertexBuffers); ++i)
	{
		m_vertexBuffers[i] = renderSystem->createVertexBuffer(
			vertexElements,
			c_glyphCount * sizeof_array(c_glyphTemplate) * sizeof(Vertex),
			true
		);
		if (!m_vertexBuffers[i])
			return false;
	}

	m_indexBuffer = renderSystem->createIndexBuffer(render::ItUInt16, c_glyphCount * 6 * sizeof(uint16_t), false);
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

	for (uint32_t i = 0; i < sizeof_array(m_vertexBuffers); ++i)
		safeDestroy(m_vertexBuffers[i]);
}

void AccGlyph::beginFrame()
{
	m_vertex = (uint8_t*)m_vertexBuffers[m_currentVertexBuffer]->lock();
	m_offset = 0;
	m_count = 0;
}

void AccGlyph::endFrame()
{
	T_FATAL_ASSERT (m_count == 0);
	m_vertexBuffers[m_currentVertexBuffer]->unlock();
	m_currentVertexBuffer = (m_currentVertexBuffer + 1) % sizeof_array(m_vertexBuffers);
}

void AccGlyph::add(
	const Aabb2& bounds,
	const Matrix33& transform,
	const Vector4& textureOffset
)
{
	T_FATAL_ASSERT (m_vertex != 0);

	if (m_count >= c_glyphCount)
	{
		log::warning << L"Too many glyphs cached; skipped" << Endl;
		return;
	}

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

	Vertex* vertex = (Vertex*)m_vertex;
	for (uint32_t i = 0; i < sizeof_array(c_glyphTemplate); ++i)
	{
		Vector4 pos = m * c_glyphTemplate[i].pos;

		vertex->pos[0] = pos.x();
		vertex->pos[1] = pos.y();
		vertex->texCoord[0] = c_glyphTemplate[i].texCoord.x;
		vertex->texCoord[1] = c_glyphTemplate[i].texCoord.y;

		vertex->texOffsetAndScale[0] = textureOffset.x();
		vertex->texOffsetAndScale[1] = textureOffset.y();

		vertex->texOffsetAndScale[2] = bounds.mx.x - bounds.mn.x;
		vertex->texOffsetAndScale[3] = bounds.mx.y - bounds.mn.y;

		vertex++;
	}

	m_vertex += sizeof(Vertex) * sizeof_array(c_glyphTemplate);
	m_count++;
}

void AccGlyph::render(
	render::RenderContext* renderContext,
	const Vector4& frameSize,
	const Vector4& viewSize,
	const Vector4& viewOffset,
	float screenOffsetScale,
	render::ITexture* texture,
	uint8_t maskReference,
	uint8_t glyphFilter,
	const SwfColor& glyphColor,
	const SwfColor& glyphFilterColor
)
{
	if (!m_count)
		return;

	render::Shader* shader = (maskReference == 0) ? m_shaderGlyph : m_shaderGlyphMask;
	T_ASSERT (shader);

	const render::handle_t techniques[] = { s_handleTechniqueDefault, s_handleTechniqueDropShadow, 0, s_handleTechniqueGlow };
	T_ASSERT (glyphFilter < sizeof_array(techniques));

	shader->setTechnique(techniques[glyphFilter]);
	if (!shader->getCurrentProgram())
		return;

	render::IndexedRenderBlock* renderBlock = renderContext->alloc< render::IndexedRenderBlock >("Flash AccGlyph");
	renderBlock->program = shader->getCurrentProgram();
	renderBlock->indexBuffer = m_indexBuffer;
	renderBlock->vertexBuffer = m_vertexBuffers[m_currentVertexBuffer];
	renderBlock->primitive = render::PtTriangles;
	renderBlock->offset = m_offset * 6;
	renderBlock->count = m_count * 2;
	renderBlock->minIndex = 0;
	renderBlock->maxIndex = c_glyphCount * 4 - 1;

	renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
	renderBlock->programParams->beginParameters(renderContext);
	renderBlock->programParams->setVectorParameter(s_handleFrameSize, frameSize);
	renderBlock->programParams->setVectorParameter(s_handleViewSize, viewSize);
	renderBlock->programParams->setVectorParameter(s_handleViewOffset, viewOffset);
	renderBlock->programParams->setFloatParameter(s_handleScreenOffsetScale, screenOffsetScale);
	renderBlock->programParams->setStencilReference(maskReference);
	renderBlock->programParams->setTextureParameter(s_handleTexture, texture);

	renderBlock->programParams->setVectorParameter(s_handleColor, Vector4(
		glyphColor.red / 255.0f,
		glyphColor.green / 255.0f,
		glyphColor.blue / 255.0f,
		glyphColor.alpha / 255.0f
	));

	if (glyphFilter != 0)
		renderBlock->programParams->setVectorParameter(s_handleFilterColor, Vector4(
			glyphFilterColor.red / 255.0f,
			glyphFilterColor.green / 255.0f,
			glyphFilterColor.blue / 255.0f,
			glyphFilterColor.alpha / 255.0f
		));

	renderBlock->programParams->endParameters(renderContext);

	renderContext->draw(render::RpOverlay, renderBlock);

	m_offset += m_count;
	m_count = 0;
}

	}
}
