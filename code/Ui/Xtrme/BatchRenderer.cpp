#include "Ui/Xtrme/BatchRenderer.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/ShaderGraph.h"
#include "Render/IProgram.h"
#include "Render/VertexElement.h"
#include "Render/VertexBuffer.h"
#include "Xml/XmlDeserializer.h"
#include "Core/Io/MemoryStream.h"

// Resources
#include "Resources/Cx2d.h"
#include "Resources/Cx2dFont.h"
#include "Resources/Cx2dImage.h"

namespace traktor
{
	namespace ui
	{
		namespace xtrme
		{
			namespace
			{

render::IProgram* createProgram(
	render::IRenderSystem* renderSystem,
	const uint8_t resource[],
	const int resourceSize
)
{
	Ref< MemoryStream > stream = gc_new< MemoryStream >((void*)resource, resourceSize, true, false);
	Ref< render::ShaderGraph > shaderGraph = xml::XmlDeserializer(stream).readObject< render::ShaderGraph >();

	T_ASSERT_M (shaderGraph, L"Unable to read shader graph");

	Ref< render::ProgramResource > programResource = renderSystem->compileProgram(shaderGraph, 4, true);
	T_ASSERT_M (programResource, L"Unable to compile shader");

	Ref< render::IProgram > program = renderSystem->createProgram(programResource);
	T_ASSERT_M (program, L"Unable to create program");

	return program;
}

			}

struct ScreenVertex
{
	Vector2 pos;
	Vector2 texCoord;
	uint32_t color;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.xtrme.BatchRenderer", BatchRenderer, Object)

BatchRenderer::BatchRenderer(render::IRenderSystem* renderSystem, render::IRenderView* renderView, uint32_t bufferCount)
:	m_renderSystem(renderSystem)
,	m_renderView(renderView)
,	m_vertexStart(0)
,	m_vertex(0)
{
	m_programs[PiDefault] = createProgram(m_renderSystem, c_ResourceCx2d, sizeof(c_ResourceCx2d));
	m_programs[PiImage] = createProgram(m_renderSystem, c_ResourceCx2dImage, sizeof(c_ResourceCx2dImage));
	m_programs[PiFont] = createProgram(m_renderSystem, c_ResourceCx2dFont, sizeof(c_ResourceCx2dFont));

	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat2, offsetof(ScreenVertex, pos)));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat2, offsetof(ScreenVertex, texCoord)));
	vertexElements.push_back(render::VertexElement(render::DuColor,	render::DtByte4N, offsetof(ScreenVertex, color)));

	m_vertexBuffer = m_renderSystem->createVertexBuffer(vertexElements, bufferCount * sizeof(ScreenVertex), true);
	T_ASSERT_M (m_vertexBuffer, L"Unable to create vertex buffer");
}

BatchRenderer::~BatchRenderer()
{
	m_vertexBuffer->destroy();
	m_vertexBuffer = 0;
}

void BatchRenderer::batch(ProgramId programId, render::ITexture* texture, render::PrimitiveType primitiveType)
{
	if (m_batches.empty())
		m_batches.push_back(Batch());

	if (m_batches.back().program != m_programs[programId] || m_batches.back().texture != texture)
	{
		if (!m_batches.back().primitives.empty())
			m_batches.push_back(Batch());

		m_batches.back().program = m_programs[programId];
		m_batches.back().texture = texture;
	}

	std::vector< render::Primitives >& primitives = m_batches.back().primitives;
	if (primitives.empty() || primitives.back().type != primitiveType || primitiveType == render::PtTriangleStrip)
	{
		primitives.push_back(render::Primitives(
			primitiveType,
			int(m_vertex - m_vertexStart),
			0
		));
	}
}

void BatchRenderer::add(int x, int y, uint32_t color)
{
	Batch& batch = m_batches.back();

	batch.primitives.back().count++;

	m_vertex->pos.x = float(x);
	m_vertex->pos.y = float(y);
	m_vertex->color = color;
	m_vertex++;
}

void BatchRenderer::add(int x, int y, float u, float v, uint32_t color)
{
	Batch& batch = m_batches.back();

	batch.primitives.back().count++;

	m_vertex->pos.x = float(x);
	m_vertex->pos.y = float(y);
	m_vertex->texCoord.x = u;
	m_vertex->texCoord.y = v;
	m_vertex->color = color;
	m_vertex++;
}

void BatchRenderer::begin()
{
	T_ASSERT_M (!m_vertexStart && !m_vertex, L"Already in begin/end section");

	m_vertexStart =
	m_vertex = static_cast< ScreenVertex* >(m_vertexBuffer->lock());
	T_ASSERT_M (m_vertex, L"Unable to lock vertex buffer");
}

void BatchRenderer::end(const Size& size)
{
	T_ASSERT_M (m_vertexStart && m_vertex, L"Not in begin/end section");

	m_vertexBuffer->unlock();

	if (m_vertex > m_vertexStart)
	{
		float iw = 1.0f / size.cx;
		float ih = 1.0f / size.cy;

		Matrix44 transform(
			iw * 2.0f,       0.0f, 0.0f, -1.0f,
			     0.0f, -ih * 2.0f, 0.0f,  1.0f,
			     0.0f,       0.0f, 1.0f,  0.0f,
			     0.0f,       0.0f, 0.0f,  1.0f
		);

		for (size_t i = 0; i < sizeof_array(m_programs); ++i)
			m_programs[i]->setMatrixParameter(L"Transform", transform);
		
		m_renderView->setVertexBuffer(m_vertexBuffer);

		for (std::vector< Batch >::iterator i = m_batches.begin(); i != m_batches.end(); ++i)
		{
			m_renderView->setProgram(i->program);

			if (i->texture)
			{
				i->program->setSamplerTexture(L"Texture", i->texture);
				i->program->setVectorParameter(L"TextureSize", Vector4(float(i->texture->getWidth()), float(i->texture->getHeight()), 0.0f, 0.0f));
			}

			// Adjust primitive counts.
			for (std::vector< render::Primitives >::iterator j = i->primitives.begin(); j != i->primitives.end(); ++j)
			{
				switch (j->type)
				{
				case render::PtLineStrip:
					j->count -= 1;
					break;
				case render::PtLines:
					j->count /= 2;
					break;
				case render::PtTriangleStrip:
					j->count -= 2;
					break;
				case render::PtTriangles:
					j->count /= 3;
					break;
				}
			}

			for (std::vector< render::Primitives >::const_iterator j = i->primitives.begin(); j != i->primitives.end(); ++j)
				m_renderView->draw(*j);
		}
	}

	m_vertexStart =
	m_vertex = 0;

	m_batches.resize(0);
}

		}
	}
}
