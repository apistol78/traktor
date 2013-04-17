#include "Core/Log/Log.h"
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
	float pos[3];
	float texCoord[2];
	uint8_t color[4];
};
#pragma pack()

const resource::Id< render::Shader > c_idShaderGlyph(Guid(L"{A8BC2D03-EB52-B744-8D4B-29E39FF0B4F5}"));
const resource::Id< render::Shader > c_idShaderGlyphMask(Guid(L"{C8FEF24B-D775-A14D-9FF3-E34A17495FB4}"));
const uint32_t c_glyphCount = 1000;
const float c_minSampleDistance = 1.0f;
const float c_maxSampleDistance = 4.0f;

const struct TemplateVertex
{
	Vector4 pos;
	Vector2 texCoord;
}
c_glyphTemplate[4] =
{
	{ Vector4(0.0f, 0.0f, 1.0f, 0.0f), Vector2(0.0f, 0.0f) },
	{ Vector4(1.0f, 0.0f, 1.0f, 0.0f), Vector2(1.0f, 0.0f) },
	{ Vector4(1.0f, 1.0f, 1.0f, 0.0f), Vector2(1.0f, 1.0f) },
	{ Vector4(0.0f, 1.0f, 1.0f, 0.0f), Vector2(0.0f, 1.0f) }
};

bool s_handleInitialized = false;
render::handle_t s_handleFrameSize;
render::handle_t s_handleViewSize;
render::handle_t s_handleViewOffset;
render::handle_t s_handleScreenOffsetScale;
render::handle_t s_handleTexture;

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
		s_handleInitialized = true;
	}

	if (!resourceManager->bind(c_idShaderGlyph, m_shaderGlyph))
		return false;

	if (!resourceManager->bind(c_idShaderGlyphMask, m_shaderGlyphMask))
		return false;

	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat3, offsetof(Vertex, pos)));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat2, offsetof(Vertex, texCoord)));
	vertexElements.push_back(render::VertexElement(render::DuColor, render::DtByte4N, offsetof(Vertex, color)));
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

void AccGlyph::add(
	const SwfRect& bounds,
	const Matrix33& transform,
	const SwfCxTransform& cxform,
	const Vector4& textureOffset
)
{
	if (m_count >= c_glyphCount)
	{
		log::warning << L"Too many glyphs cached; skipped" << Endl;
		return;
	}

	if (!m_vertex)
	{
		m_vertex = (uint8_t*)m_vertexBuffers[m_currentVertexBuffer]->lock();
		m_count = 0;
	}

	if (!m_vertex)
		return;

	Matrix44 m1(
		transform.e11, transform.e12, std::floor(transform.e13), 0.0f,
		transform.e21, transform.e22, std::floor(transform.e23), 0.0f,
		transform.e31, transform.e32, std::floor(transform.e33), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	SwfRect qb;
	qb.min.x = std::ceil(bounds.min.x);
	qb.min.y = std::ceil(bounds.min.y);
	qb.max.x = std::floor(bounds.max.x);
	qb.max.y = std::floor(bounds.max.y);

	Matrix44 m2(
		qb.max.x - qb.min.x, 0.0f, qb.min.x, 0.0f,
		0.0f, qb.max.y - qb.min.y, qb.min.y, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	Matrix44 m = m1 * m2;
	Vector2 texCoordOffset(textureOffset.x(), textureOffset.y());
	Vector2 texCoordScale(textureOffset.z(), textureOffset.w());

	uint8_t color[4] =
	{
		uint8_t(cxform.red[0] * 255),
		uint8_t(cxform.green[0] * 255),
		uint8_t(cxform.blue[0] * 255),
		uint8_t(cxform.alpha[0] * 255)
	};

	float sampleDistance = clamp(1.0f / (m1(0, 0) * 11.0f), c_minSampleDistance, c_maxSampleDistance);

	Vertex* vertex = (Vertex*)m_vertex;
	for (uint32_t i = 0; i < sizeof_array(c_glyphTemplate); ++i)
	{
		Vector4 pos = m * c_glyphTemplate[i].pos;
		Vector2 texCoord = c_glyphTemplate[i].texCoord * texCoordScale + texCoordOffset;

		vertex->pos[0] = std::floor(pos.x());
		vertex->pos[1] = std::floor(pos.y());
		vertex->pos[2] = sampleDistance;
		vertex->texCoord[0] = texCoord.x;
		vertex->texCoord[1] = texCoord.y;
		vertex->color[0] = color[0];
		vertex->color[1] = color[1];
		vertex->color[2] = color[2];
		vertex->color[3] = color[3];

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
	uint8_t maskReference
)
{
	if (!m_vertex || !m_count)
		return;

	render::VertexBuffer* vertexBuffer = m_vertexBuffers[m_currentVertexBuffer];

	vertexBuffer->unlock();

	render::IndexedRenderBlock* renderBlock = renderContext->alloc< render::IndexedRenderBlock >("Flash AccGlyph");
	renderBlock->program = ((maskReference == 0) ? m_shaderGlyph : m_shaderGlyphMask)->getCurrentProgram();
	renderBlock->indexBuffer = m_indexBuffer;
	renderBlock->vertexBuffer = vertexBuffer;
	renderBlock->primitive = render::PtTriangles;
	renderBlock->offset = 0;
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

	renderBlock->programParams->endParameters(renderContext);

	renderContext->draw(render::RpOverlay, renderBlock);

	m_currentVertexBuffer = (m_currentVertexBuffer + 1) % sizeof_array(m_vertexBuffers);
	m_vertex = 0;
	m_count = 0;
}

	}
}
