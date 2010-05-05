#include <limits>
#include <algorithm>
#include "Spray/PointRenderer.h"
#include "Render/IRenderSystem.h"
#include "Render/VertexElement.h"
#include "Render/VertexBuffer.h"
#include "Render/IndexBuffer.h"
#include "Render/Context/RenderContext.h"
#include "World/WorldRenderView.h"
#include "Core/Math/Const.h"

namespace traktor
{
	namespace spray
	{
		namespace
		{

#if TARGET_OS_MAC
const uint32_t c_pointCount = 3000;
#elif TARGET_OS_IPHONE
const uint32_t c_pointCount = 1000;
#elif defined(_WINCE)
const uint32_t c_pointCount = 1000;
#else
const uint32_t c_pointCount = 4000;
#endif

struct PredicateZ
{
	bool operator () (const std::pair< uint32_t, float >& i1, const std::pair< uint32_t, float >& i2) const
	{
		return bool(i1.second > i2.second);
	}
};

		}

#pragma pack(1)
struct Vertex
{
	struct PositionAndOrientation
	{
		float position[3];
		float orientation;
	}
	positionAndOrientation;
	
	struct VelocityAndRandom
	{
		float velocity[3];
		float random;
	}
	velocityAndRandom;

	struct Attributes1
	{
		float extent[2];
		float alpha;
		float size;
	}
	attrib1;

	struct Attributes2
	{
		float color[3];
		float age;
	}
	attrib2;
};
#pragma pack()

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.PointRenderer", PointRenderer, Object)

PointRenderer::PointRenderer(render::IRenderSystem* renderSystem, float cullNearDistance, float fadeNearRange)
:	m_cullNearDistance(cullNearDistance)
,	m_fadeNearRange(fadeNearRange)
,	m_currentBuffer(0)
,	m_vertex(0)
,	m_vertexOffset(0)
{
	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat4, offsetof(Vertex, positionAndOrientation), 0));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat4, offsetof(Vertex, velocityAndRandom), 0));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat4, offsetof(Vertex, attrib1), 1));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat4, offsetof(Vertex, attrib2), 2));
	T_ASSERT_M (render::getVertexSize(vertexElements) == sizeof(Vertex), L"Incorrect size of vertex");

	for (int i = 0; i < BufferCount; ++i)
	{
		m_vertexBuffer[i] = renderSystem->createVertexBuffer(vertexElements, c_pointCount * 4 * sizeof(Vertex), true);
		T_ASSERT_M (m_vertexBuffer[i], L"Unable to create vertex buffer");
	}

	m_indexBuffer = renderSystem->createIndexBuffer(render::ItUInt16, c_pointCount * 3 * 2 * sizeof(uint16_t), false);
	T_ASSERT_M (m_indexBuffer, L"Unable to create index buffer");

	uint16_t* index = static_cast< uint16_t* >(m_indexBuffer->lock());
	for (uint32_t i = 0; i < c_pointCount * 4; i += 4)
	{
		*index++ = i + 0;
		*index++ = i + 1;
		*index++ = i + 2;
		*index++ = i + 0;
		*index++ = i + 2;
		*index++ = i + 3;
	}

	m_indexBuffer->unlock();
}

void PointRenderer::render(
	render::Shader* shader,
	const Plane& cameraPlane,
	const PointVector& points,
	float middleAge
)
{
	uint32_t pointOffset = m_vertexOffset >> 2;

	uint32_t size = uint32_t(points.size());
	T_ASSERT (size > 0);

	size = std::min(size, c_pointCount - pointOffset);
	if (!size)
		return;

	AlignedVector< Batch >& batches = m_batches[m_currentBuffer];

	batches.push_back(Batch());
	batches.back().shader = shader;
	batches.back().offset = pointOffset * 3 * 2;
	batches.back().count = 0;
	batches.back().distance = std::numeric_limits< float >::max();

	const float c_extents[4][2] =
	{
		{ -1.0f,  1.0f },
		{  1.0f,  1.0f },
		{  1.0f, -1.0f },
		{ -1.0f, -1.0f }
	};

	//static std::vector< std::pair< uint32_t, float > > sorted;
	//sorted.resize(size);

	//for (uint32_t i = 0; i < size; ++i)
	//{
	//	sorted[i].first = i;
	//	sorted[i].second = cameraPlane.distance(points[i].position);
	//}

	//std::sort(sorted.begin(), sorted.end(), PredicateZ());

	if (!m_vertex)
	{
		m_vertex = static_cast< Vertex* >(m_vertexBuffer[m_currentBuffer]->lock());
		if (!m_vertex)
			return;
	}

	//for (std::vector< std::pair< uint32_t, float > >::iterator i = sorted.begin(); i != sorted.end(); ++i)
	//{
	//	// Skip particles behind near culling plane.
	//	float distance = i->second;
	//	if (distance < m_cullNearDistance)
	//		continue;

	//	const Point& point = points[i->first];

	for (uint32_t i = 0; i < size; ++i)
	{
		const Point& point = points[i];

		float distance = cameraPlane.distance(point.position);
		if (distance < m_cullNearDistance)
			continue;

		// Calculate alpha based on point age and distance from near culling plane.
		float age = clamp(point.age / point.maxAge, 0.0f, 1.0f);
		float middle = age - middleAge;
		float alpha = select(middle, 1.0f - middle / (1.0f - middleAge), age / middleAge);
		alpha *= min((distance - m_cullNearDistance) / m_fadeNearRange, 1.0f);
		if (alpha < FUZZY_EPSILON)
			continue;

		for (int j = 0; j < 4; ++j)
		{
			point.position.storeUnaligned(m_vertex->positionAndOrientation.position);
			point.velocity.storeUnaligned(m_vertex->velocityAndRandom.velocity);
			point.color.storeUnaligned(m_vertex->attrib2.color);

			m_vertex->positionAndOrientation.orientation = point.orientation;
			m_vertex->velocityAndRandom.random = point.random;
			m_vertex->attrib1.extent[0] = c_extents[j][0];
			m_vertex->attrib1.extent[1] = c_extents[j][1];
			m_vertex->attrib1.alpha = alpha;
			m_vertex->attrib1.size = point.size;
			m_vertex->attrib2.age = age;

			m_vertex++;
		}

		batches.back().distance = min(batches.back().distance, distance);
		batches.back().count += 2;

		m_vertexOffset += 4;
	}
}

void PointRenderer::flush(
	render::RenderContext* renderContext,
	world::WorldRenderView* worldRenderView
)
{
	if (m_vertexOffset > 0)
	{
		T_ASSERT (m_vertex);

		m_vertex = 0;
		m_vertexBuffer[m_currentBuffer]->unlock();

		for (AlignedVector< Batch >::iterator i = m_batches[m_currentBuffer].begin(); i != m_batches[m_currentBuffer].end(); ++i)
		{
			if (!i->shader || !i->count)
				continue;

			render::IndexedRenderBlock* renderBlock = renderContext->alloc< render::IndexedRenderBlock >("PointRenderer");

			renderBlock->distance = i->distance;
			renderBlock->shader = i->shader;
			renderBlock->shaderParams = renderContext->alloc< render::ShaderParameters >();
			renderBlock->indexBuffer = m_indexBuffer;
			renderBlock->vertexBuffer = m_vertexBuffer[m_currentBuffer];
			renderBlock->primitive = render::PtTriangles;
			renderBlock->offset = i->offset;
			renderBlock->count = i->count;
			renderBlock->minIndex = 0;
			renderBlock->maxIndex = m_vertexOffset;

			renderBlock->shaderParams->beginParameters(renderContext);
			worldRenderView->setShaderParameters(renderBlock->shaderParams);
			renderBlock->shaderParams->endParameters(renderContext);

			renderContext->draw(render::RfAlphaBlend, renderBlock);
		}

		m_batches[m_currentBuffer].resize(0);
		m_currentBuffer = (m_currentBuffer + 1) % BufferCount;
		m_vertexOffset = 0;
	}
	else
		m_batches[m_currentBuffer].resize(0);
}

	}
}
