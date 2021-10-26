#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Format.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
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

const uint32_t c_stripeLength = 64;
const uint32_t c_trailCount = 16;

render::handle_t s_handleTimeAndAge = 0;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.TrailRenderer", TrailRenderer, Object)

TrailRenderer::TrailRenderer(render::IRenderSystem* renderSystem)
:	m_count(0)
,	m_vertex(nullptr)
{
	s_handleTimeAndAge = render::getParameterHandle(L"TimeAndAge");

	AlignedVector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat4, offsetof(TrailVertex, position), 0));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat4, offsetof(TrailVertex, direction), 0));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat4, offsetof(TrailVertex, uv), 1));
	T_ASSERT_M (render::getVertexSize(vertexElements) == sizeof(TrailVertex), L"Incorrect size of vertex");
	m_vertexLayout = renderSystem->createVertexLayout(vertexElements);

	for (uint32_t i = 0; i < sizeof_array(m_vertexBuffers); ++i)
	{
		m_vertexBuffers[i] = renderSystem->createBuffer(render::BuVertex, c_trailCount * c_stripeLength * 2, sizeof(TrailVertex), true);
		T_ASSERT_M (m_vertexBuffers[i], L"Unable to create vertex buffer");
	}

	m_indexBuffer = renderSystem->createBuffer(render::BuIndex, c_trailCount * c_stripeLength * 2, sizeof(uint16_t), false);
	T_ASSERT_M (m_indexBuffer, L"Unable to create index buffer");

	uint16_t* index = static_cast< uint16_t* >(m_indexBuffer->lock());
	for (uint32_t i = 0; i < c_trailCount * c_stripeLength * 2; ++i)
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
	const CircularVector< Vector4, 64 >& points,
	const Vector4& cameraPosition,
	const Plane& cameraPlane,
	float width,
	float time,
	float age
)
{
	const int32_t pointCount = (int32_t)points.size();
	if (pointCount < 2)
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
	m_batches.back().timeAndAge = Vector4(time, age, 0.0f, 0.0f);

	// Calculate total length so we can properly distribute alpha.
	Scalar tln = 0.0_simd;
	for (int32_t i = pointCount - 1; i >= 1; --i)
	{
		Vector4 vp0 = points[i - 1];
		Vector4 vp1 = points[i];
		tln += (vp1 - vp0).xyz0().length();
	}

	TrailVertex* vertex = m_vertex;

	const Vector4 www0(width, width, width, 0.0f);
	float v = 0.0f;

	for (int32_t i = pointCount - 1; i > 1; --i)
	{
		Vector4 vp0 = points[i - 1];
		Vector4 vp1 = points[i];

		Vector4 direction = (vp1 - vp0).xyz0();
		Scalar ln = direction.length2();
		if (ln <= FUZZY_EPSILON * FUZZY_EPSILON)
			continue;

		Vector4 up = cross(direction, (vp0 + vp1) * Scalar(0.5f) - cameraPosition).normalized() * www0;

		vp1.storeAligned(vertex->position);
		up.storeAligned(vertex->direction);
		vertex->uv[0] = 0.0f;
		vertex->uv[1] = v;
		vertex->uv[2] = 1.0f - v / tln;
		++vertex;

		vp1.storeAligned(vertex->position);
		(-up).storeAligned(vertex->direction);
		vertex->uv[0] = 1.0f;
		vertex->uv[1] = v;
		vertex->uv[2] = 1.0f - v / tln;
		++vertex;

		v += squareRoot(ln);

		m_batches.back().points++;
	}

	// Add tail.
	{
		Vector4 vp0 = points[0];
		Vector4 vp1 = points[1];

		Vector4 direction = (vp1 - vp0).xyz0();
		Scalar ln = direction.length2();
		if (ln > FUZZY_EPSILON * FUZZY_EPSILON)
		{
			direction *= reciprocalSquareRoot(ln);

			Scalar k = clamp(1.0_simd + (Scalar(time - age) - vp0.w()) / (vp1.w() - vp0.w()), 0.0_simd, 1.0_simd);
			Vector4 vp = lerp(vp0, vp1, k);
			Vector4 up = cross(direction, vp - cameraPosition).normalized() * www0;

			vp.storeAligned(vertex->position);
			up.storeAligned(vertex->direction);
			vertex->uv[0] = 0.0f;
			vertex->uv[1] = v;
			vertex->uv[2] = 0.0f;
			++vertex;

			vp.storeAligned(vertex->position);
			(-up).storeAligned(vertex->direction);
			vertex->uv[0] = 1.0f;
			vertex->uv[1] = v;
			vertex->uv[2] = 0.0f;
			++vertex;

			m_batches.back().points++;
		}
	}

	m_vertex += c_stripeLength * 2;
}

void TrailRenderer::flush(
	render::RenderContext* renderContext,
	const world::IWorldRenderPass& worldRenderPass
)
{
	if (!m_vertex)
		return;

	m_vertex = nullptr;
	m_vertexBuffers[m_count]->unlock();

	uint32_t offset = 0;

	for (const auto& batch : m_batches)
	{
		if (!batch.shader || !batch.points)
			continue;

		auto sp = worldRenderPass.getProgram(batch.shader);
		if (!sp)
			continue;

		render::IndexedRenderBlock* renderBlock = renderContext->alloc< render::IndexedRenderBlock >(L"Trail");

		renderBlock->distance = -std::numeric_limits< float >::max();
		renderBlock->program = sp.program;
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->indexBuffer = m_indexBuffer->getBufferView();
		renderBlock->indexType = render::IndexType::UInt16;
		renderBlock->vertexBuffer = m_vertexBuffers[m_count]->getBufferView();
		renderBlock->vertexLayout = m_vertexLayout;
		renderBlock->primitive = render::PrimitiveType::TriangleStrip;
		renderBlock->offset = offset;
		renderBlock->count = batch.points * 2 - 2;
		renderBlock->minIndex = 0;
		renderBlock->maxIndex = c_trailCount * c_stripeLength * 2;

		renderBlock->programParams->beginParameters(renderContext);
		worldRenderPass.setProgramParameters(renderBlock->programParams);
		renderBlock->programParams->setVectorParameter(s_handleTimeAndAge, batch.timeAndAge);
		renderBlock->programParams->endParameters(renderContext);

		renderContext->draw(sp.priority, renderBlock);

		offset += c_stripeLength * 2;
	}

	m_count = (m_count + 1) % sizeof_array(m_vertexBuffers);
	m_batches.resize(0);
}

	}
}
