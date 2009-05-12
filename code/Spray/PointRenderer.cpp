#include <limits>
#include <algorithm>
#include "Spray/PointRenderer.h"
#include "Render/RenderSystem.h"
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

const uint32_t c_pointCount = 2000;

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
	float position[3];
	float velocity[3];
	struct Attributes1
	{
		float extent[2];
		float alpha;
		float size;
	}
	attrib1;
	struct Attributes2
	{
		float orientation;
		float random;
		float dummy[2];
	}
	attrib2;
};
#pragma pack()

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.PointRenderer", PointRenderer, Object)

PointRenderer::PointRenderer(render::RenderSystem* renderSystem, float cullNearDistance, float fadeNearRange)
:	m_cullNearDistance(cullNearDistance)
,	m_fadeNearRange(fadeNearRange)
,	m_currentBuffer(0)
,	m_vertexOffset(0)
{
	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat3, offsetof(Vertex, position), 0));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat3, offsetof(Vertex, velocity), 0));
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
	if (pointOffset >= c_pointCount)
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

	uint32_t size = uint32_t(points.size());
	T_ASSERT (size > 0);

	static std::vector< std::pair< uint32_t, float > > sorted;
	sorted.resize(size);

	for (uint32_t i = 0; i < size; ++i)
	{
		sorted[i].first = i;
		sorted[i].second = cameraPlane.distance(points[i].position);
	}

	std::sort(sorted.begin(), sorted.end(), PredicateZ());

	Vertex* vertex = static_cast< Vertex* >(m_vertexBuffer[m_currentBuffer]->lock(m_vertexOffset, size * 4));
	if (vertex)
	{
		for (std::vector< std::pair< uint32_t, float > >::iterator i = sorted.begin(); i != sorted.end(); ++i)
		{
			// Skip particles behind near culling plane.
			if (i->second < m_cullNearDistance)
				continue;

			const Point& point = points[i->first];

			// Calculate alpha based on point age and distance from near culling plane.
			float age = point.age / point.maxAge;
			float middle = age - middleAge;
			float alpha = select(middle, 1.0f - middle / (1.0f - middleAge), age / middleAge);
			alpha *= min((i->second - m_cullNearDistance) / m_fadeNearRange, 1.0f);
			if (alpha < FUZZY_EPSILON)
				continue;

			for (int j = 0; j < 4; ++j)
			{
				vertex->position[0] = point.position.x();
				vertex->position[1] = point.position.y();
				vertex->position[2] = point.position.z();
				vertex->velocity[0] = point.velocity.x();
				vertex->velocity[1] = point.velocity.y();
				vertex->velocity[2] = point.velocity.z();
				vertex->attrib1.extent[0] = c_extents[j][0];
				vertex->attrib1.extent[1] = c_extents[j][1];
				vertex->attrib1.alpha = alpha;
				vertex->attrib1.size = point.size;
				vertex->attrib2.orientation = point.orientation;
				vertex->attrib2.random = point.random;
				vertex++;
			}

			batches.back().distance = min(batches.back().distance, i->second);
			batches.back().count += 2;

			m_vertexOffset += 4;
		}
		m_vertexBuffer[m_currentBuffer]->unlock();
	}
}

void PointRenderer::flush(
	render::RenderContext* renderContext,
	world::WorldRenderView* worldRenderView
)
{
	if (m_vertexOffset > 0)
	{
		for (AlignedVector< Batch >::iterator i = m_batches[m_currentBuffer].begin(); i != m_batches[m_currentBuffer].end(); ++i)
		{
			if (!i->shader || !i->count)
				continue;

			render::IndexedRenderBlock* renderBlock = renderContext->alloc< render::IndexedRenderBlock >();

			renderBlock->type = render::RbtAlphaBlend;
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

			renderContext->draw(renderBlock);
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
