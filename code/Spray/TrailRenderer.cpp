#include <limits>
#include "Core/Math/Const.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/IRenderSystem.h"
#include "Render/IndexBuffer.h"
#include "Render/Shader.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"
#include "Spray/TrailRenderer.h"
#include "Spray/Vertex.h"
#include "World/IWorldRenderPass.h"

namespace traktor
{
	namespace spray
	{
		namespace
		{

const uint32_t c_stripeCount = 16;
const uint32_t c_trailCount = 8;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.TrailRenderer", TrailRenderer, Object)

TrailRenderer::TrailRenderer(render::IRenderSystem* renderSystem)
:	m_count(0)
,	m_vertex(0)
{
	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat4, offsetof(TrailVertex, position), 0));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat4, offsetof(TrailVertex, uv), 0));
	T_ASSERT_M (render::getVertexSize(vertexElements) == sizeof(TrailVertex), L"Incorrect size of vertex");

	for (uint32_t i = 0; i < sizeof_array(m_vertexBuffers); ++i)
	{
		m_vertexBuffers[i] = renderSystem->createVertexBuffer(vertexElements, c_trailCount * c_stripeCount * 2 * sizeof(TrailVertex), true);
		T_ASSERT_M (m_vertexBuffers[i], L"Unable to create vertex buffer");
	}

	m_indexBuffer = renderSystem->createIndexBuffer(render::ItUInt16, c_trailCount * c_stripeCount * 3 * 2 * sizeof(uint16_t), false);
	T_ASSERT_M (m_indexBuffer, L"Unable to create index buffer");

	uint16_t* index = static_cast< uint16_t* >(m_indexBuffer->lock());
	for (uint32_t i = 0; i < c_trailCount * c_stripeCount * 2; ++i)
		*index++ = i;

	m_indexBuffer->unlock();
}

TrailRenderer::~TrailRenderer()
{
	destroy();
}

void TrailRenderer::destroy()
{
	safeDestroy(m_indexBuffer);

	for (uint32_t i = 0; i < sizeof_array(m_vertexBuffers); ++i)
		safeDestroy(m_vertexBuffers[i]);
}

void TrailRenderer::render(
	render::Shader* shader,
	const CircularVector< Vector4, 16 >& points,
	const Vector4& cameraPosition,
	const Plane& cameraPlane,
	float width,
	float lengthTreshold
)
{
	int32_t pointCount = int32_t(points.size());
	if (pointCount < 1)
		return;

	if (m_batches.size() >= c_trailCount)
		return;

	if (!m_vertex)
	{
		m_vertex = static_cast< TrailVertex* >(m_vertexBuffers[m_count]->lock());
		if (!m_vertex)
			return;
	}

	m_batches.push_back(Batch());
	m_batches.back().shader = shader;
	m_batches.back().points = 0;

	TrailVertex* vertex = m_vertex;

	const Scalar w(width);
	float v = 0.0f;

	for (int32_t i = pointCount - 1; i >= 1; --i)
	{
		Vector4 vp0 = points[i - 1];
		Vector4 vp1 = points[i];

		Vector4 direction = (vp1 - vp0).xyz0();
		Vector4 up = Vector4::zero();

		Scalar ln = direction.length();
		if (ln <= FUZZY_EPSILON)
			continue;

		direction /= ln;
		up = cross(direction, (vp0 + vp1) * Scalar(0.5f) - cameraPosition).normalized() * w;

		(vp1 + up).storeUnaligned(vertex->position);
		vertex->uv[0] = 0.0f;
		vertex->uv[1] = v;
		vertex->uv[2] = (i > 1) ? 1.0f : 0.0f;
		++vertex;

		(vp1 - up).storeUnaligned(vertex->position);
		vertex->uv[0] = 1.0f;
		vertex->uv[1] = v;
		vertex->uv[2] = (i > 1) ? 1.0f : 0.0f;
		++vertex;

		v += ln / (lengthTreshold * (c_stripeCount - 3));

		m_batches.back().points++;
	}

	m_vertex += c_stripeCount * 2;
}

void TrailRenderer::flush(
	render::RenderContext* renderContext,
	world::IWorldRenderPass& worldRenderPass
)
{
	if (!m_vertex)
		return;

	m_vertex = 0;
	m_vertexBuffers[m_count]->unlock();

	uint32_t offset = 0;

	for (AlignedVector< Batch >::iterator i = m_batches.begin(); i != m_batches.end(); ++i)
	{
		if (!i->shader || !i->points)
			continue;

		worldRenderPass.setShaderTechnique(i->shader);
		worldRenderPass.setShaderCombination(i->shader);

		render::IProgram* program = i->shader->getCurrentProgram();
		if (!program)
			continue;

		render::IndexedRenderBlock* renderBlock = renderContext->alloc< render::IndexedRenderBlock >("Trail");

		renderBlock->distance = -std::numeric_limits< float >::max();
		renderBlock->program = program;
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->indexBuffer = m_indexBuffer;
		renderBlock->vertexBuffer = m_vertexBuffers[m_count];
		renderBlock->primitive = render::PtTriangleStrip;
		renderBlock->offset = offset;
		renderBlock->count = i->points * 2 - 2;
		renderBlock->minIndex = 0;
		renderBlock->maxIndex = c_trailCount * c_stripeCount * 2;

		renderBlock->programParams->beginParameters(renderContext);
		worldRenderPass.setProgramParameters(renderBlock->programParams, false);
		renderBlock->programParams->endParameters(renderContext);

		renderContext->draw(i->shader->getCurrentPriority(), renderBlock);

		offset += c_stripeCount * 2;
	}

	m_count = (m_count + 1) % sizeof_array(m_vertexBuffers);
	m_batches.resize(0);
}

	}
}
