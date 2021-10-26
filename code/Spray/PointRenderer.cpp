#include <algorithm>
#include <limits>
#include "Core/Math/Const.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"
#include "Spray/PointRenderer.h"
#include "Spray/Vertex.h"
#include "World/IWorldRenderPass.h"

namespace traktor
{
	namespace spray
	{
		namespace
		{

#if defined(__IOS__) || defined(__ANDROID__)
const uint32_t c_pointCount = 1000;
#else
const uint32_t c_pointCount = 8000;
#endif

const static float c_extents[4][2] =
{
	{ -1.0f, -1.0f },
	{  1.0f, -1.0f },
	{  1.0f,  1.0f },
	{ -1.0f,  1.0f }
};

const render::Handle s_handlePoints(L"Spray_Points");
const render::Handle s_handlePointsOffset(L"Spray_PointsOffset");

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.PointRenderer", PointRenderer, Object)

PointRenderer::PointRenderer(render::IRenderSystem* renderSystem, float lod1Distance, float lod2Distance)
:	m_lod1Distance(lod1Distance)
,	m_lod2Distance(lod2Distance)
,	m_point(nullptr)
,	m_pointOffset(0)
{
	AlignedVector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat4, offsetof(EmitterVertex, extent), 0));
	T_ASSERT_M (render::getVertexSize(vertexElements) == sizeof(EmitterVertex), L"Incorrect size of vertex");
	m_vertexLayout = renderSystem->createVertexLayout(vertexElements);

	m_vertexBuffer = renderSystem->createBuffer(render::BuVertex, 4, sizeof(EmitterVertex), false);
	T_ASSERT_M (m_vertexBuffer, L"Unable to create vertex buffer");

	EmitterVertex* vertex = (EmitterVertex*)m_vertexBuffer->lock();
	for (int32_t i = 0; i < 4; ++i)
	{
		vertex->extent[0] = c_extents[i][0];
		vertex->extent[1] = c_extents[i][1];
		vertex->extent[2] = 0.0f;
		vertex->extent[3] = 0.0f;
		vertex++;
	}
	m_vertexBuffer->unlock();

	m_indexBuffer = renderSystem->createBuffer(render::BuIndex, 6, sizeof(uint16_t), false);
	T_ASSERT_M (m_indexBuffer, L"Unable to create index buffer");

	uint16_t* index = (uint16_t*)m_indexBuffer->lock();
	*index++ = 0;
	*index++ = 1;
	*index++ = 2;
	*index++ = 0;
	*index++ = 2;
	*index++ = 3;
	m_indexBuffer->unlock();

	m_structBuffer = renderSystem->createBuffer(render::BuStructured, c_pointCount, sizeof(EmitterPoint), true);
	T_ASSERT_M (m_structBuffer, L"Unable to create struct buffer");
}

PointRenderer::~PointRenderer()
{
	destroy();
}

void PointRenderer::destroy()
{
	if (m_point)
	{
		m_structBuffer->unlock();
		m_point = nullptr;
	}

	safeDestroy(m_indexBuffer);
	safeDestroy(m_vertexBuffer);
	safeDestroy(m_structBuffer);
}

void PointRenderer::render(
	render::Shader* shader,
	const Plane& cameraPlane,
	const PointVector& points,
	float middleAge,
	float cullNearDistance,
	float fadeNearRange,
	float cameraOffset
)
{
	int32_t size = (int32_t)points.size();
	T_ASSERT(size > 0);

	int32_t avail = c_pointCount - m_pointOffset;
	if (avail <= 0)
		return;

	if (size > avail)
		size = avail;

	if (!m_point)
	{
		m_point = (EmitterPoint*)m_structBuffer->lock();
		if (!m_point)
			return;
	}

	Batch& back = m_batches.push_back();
	back.shader = shader;
	back.offset = m_pointOffset;
	back.count = 0;
	back.distance = std::numeric_limits< float >::max();

	const Vector4 cameraOffsetV = cameraPlane.normal() * Scalar(cameraOffset);

	for (int32_t i = 0; i < size; ++i)
	{
		const Point& point = points[i];

		// Skip very small particles.
		if (point.size < FUZZY_EPSILON)
			continue;

		float distance = cameraPlane.distance(point.position);

		// Skip particles if closer than near cull distance.
		if (distance < cullNearDistance)
			continue;

		// Calculate alpha based on point age and distance from near culling plane.
		float age = clamp(point.age / point.maxAge, 0.0f, 1.0f);
		float middle = age - middleAge;
		float alpha = select(middle, 1.0f - middle / (1.0f - middleAge), age / middleAge);
		alpha *= min((distance - cullNearDistance) / fadeNearRange, 1.0f);
		if (alpha < FUZZY_EPSILON)
			continue;

		Vector4 position = point.position + cameraOffsetV;

		// \note We're assuming locked vertex buffer is 16-aligned.
		position.storeAligned(m_point->positionAndOrientation);
		point.velocity.storeAligned(m_point->velocityAndRandom);
		point.color.storeAligned(m_point->colorAndAge);
		m_point->positionAndOrientation[3] = point.orientation;
		m_point->velocityAndRandom[3] = point.random;
		m_point->alphaAndSize[0] = alpha;
		m_point->alphaAndSize[1] = point.size;
		m_point->colorAndAge[3] = age;
		m_point++;

		back.distance = min(back.distance, distance);
		back.count++;

		m_pointOffset++;
	}
}

void PointRenderer::flush(
	render::RenderContext* renderContext,
	const world::IWorldRenderPass& worldRenderPass
)
{
	if (m_pointOffset > 0)
	{
		m_structBuffer->unlock();

		for (const auto& batch : m_batches)
		{
			if (!batch.count || !batch.shader)
				continue;

			auto sp = worldRenderPass.getProgram(batch.shader);
			if (!sp)
				continue;

			auto renderBlock = renderContext->alloc< render::IndexedInstancingRenderBlock >(T_FILE_LINE_W);

			renderBlock->distance = batch.distance;
			renderBlock->program = sp.program;
			renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
			renderBlock->indexBuffer = m_indexBuffer->getBufferView();
			renderBlock->indexType = render::IndexType::UInt16;
			renderBlock->vertexBuffer = m_vertexBuffer->getBufferView();
			renderBlock->vertexLayout = m_vertexLayout;
			renderBlock->primitive = render::PrimitiveType::Triangles;
			renderBlock->offset = 0;
			renderBlock->count = 2;
			renderBlock->minIndex = 0;
			renderBlock->maxIndex = 3;
			renderBlock->instanceCount = batch.count;

			renderBlock->programParams->beginParameters(renderContext);
			worldRenderPass.setProgramParameters(renderBlock->programParams);
			renderBlock->programParams->setFloatParameter(s_handlePointsOffset, batch.offset);
			renderBlock->programParams->setBufferViewParameter(s_handlePoints, m_structBuffer->getBufferView());
			renderBlock->programParams->endParameters(renderContext);

			renderContext->draw(
				sp.priority,
				renderBlock
			);
		}

		m_point = nullptr;
		m_pointOffset = 0;
	}

	m_batches.resize(0);
}

	}
}
