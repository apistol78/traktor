#include <algorithm>
#include <limits>
#include "Core/Math/Const.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/IRenderSystem.h"
#include "Render/VertexElement.h"
#include "Render/VertexBuffer.h"
#include "Render/IndexBuffer.h"
#include "Render/Context/RenderContext.h"
#include "Spray/PointRenderer.h"
#include "Spray/Vertex.h"
#include "World/IWorldRenderPass.h"

#if defined(_PS3)
#	include "Core/Thread/Ps3/Spurs/SpursJobQueue.h"
#	include "Core/Thread/Ps3/Spurs/SpursManager.h"
#	include "Spray/Ps3/Spu/JobPointRenderer.h"

#	if !defined(_DEBUG)

extern char _binary_jqjob_Traktor_Spray_JobPointRenderer_bin_start[];
extern char _binary_jqjob_Traktor_Spray_JobPointRenderer_bin_size[];

static char* job_start = _binary_jqjob_Traktor_Spray_JobPointRenderer_bin_start;
static char* job_size = _binary_jqjob_Traktor_Spray_JobPointRenderer_bin_size;

#	else

extern char _binary_jqjob_Traktor_Spray_JobPointRenderer_d_bin_start[];
extern char _binary_jqjob_Traktor_Spray_JobPointRenderer_d_bin_size[];

static char* job_start = _binary_jqjob_Traktor_Spray_JobPointRenderer_d_bin_start;
static char* job_size = _binary_jqjob_Traktor_Spray_JobPointRenderer_d_bin_size;

#	endif
#endif

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
#elif defined(_PS3)
const uint32_t c_pointCount = 3000;
#else
const uint32_t c_pointCount = 7000;
#endif

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.PointRenderer", PointRenderer, Object)

PointRenderer::PointRenderer(render::IRenderSystem* renderSystem, float lod1Distance, float lod2Distance)
:	m_lod1Distance(lod1Distance)
,	m_lod2Distance(lod2Distance)
,	m_count(0)
,	m_vertex(0)
,	m_vertexOffset(0)
{
	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat4, offsetof(EmitterVertex, positionAndOrientation), 0));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat4, offsetof(EmitterVertex, velocityAndRandom), 0));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat4, offsetof(EmitterVertex, attrib1), 1));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat4, offsetof(EmitterVertex, attrib2), 2));
	T_ASSERT_M (render::getVertexSize(vertexElements) == sizeof(EmitterVertex), L"Incorrect size of vertex");

	for (uint32_t i = 0; i < sizeof_array(m_vertexBuffers); ++i)
	{
		m_vertexBuffers[i] = renderSystem->createVertexBuffer(vertexElements, c_pointCount * 4 * sizeof(EmitterVertex), true);
		T_ASSERT_M (m_vertexBuffers[i], L"Unable to create vertex buffer");
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

#if defined(_PS3)
	m_jobQueue = SpursManager::getInstance().createJobQueue(sizeof(JobPointRenderer), 256, SpursManager::Normal);
#endif
}

PointRenderer::~PointRenderer()
{
	destroy();
}

void PointRenderer::destroy()
{
#if defined(_PS3)
	safeDestroy(m_jobQueue);
#endif

	safeDestroy(m_indexBuffer);

	for (uint32_t i = 0; i < sizeof_array(m_vertexBuffers); ++i)
		safeDestroy(m_vertexBuffers[i]);
}

void PointRenderer::render(
	render::Shader* shader,
	const Plane& cameraPlane,
	const PointVector& points,
	float middleAge,
	float cullNearDistance,
	float fadeNearRange
)
{
	int32_t pointOffset = m_vertexOffset >> 2;

	int32_t size = int32_t(points.size());
	T_ASSERT (size > 0);

	int32_t avail = c_pointCount - pointOffset;

	size = std::min(size, avail);
	if (size <= 0)
		return;

	if (!m_vertex)
	{
		m_vertex = static_cast< EmitterVertex* >(m_vertexBuffers[m_count]->lock());
		if (!m_vertex)
			return;
	}

	AlignedVector< Batch >& batches = m_batches;

	batches.push_back(Batch());
	batches.back().shader = shader;
	batches.back().offset = pointOffset * 3 * 2;
	batches.back().count = 0;
	batches.back().distance = std::numeric_limits< float >::max();

#if defined(_PS3)

	JobPointRenderer job;

	__builtin_memset(&job, 0, sizeof(JobPointRenderer));
	job.header.eaBinary = (uintptr_t)job_start;
	job.header.sizeBinary = CELL_SPURS_GET_SIZE_BINARY(job_size);

	cameraPlane.normal().storeAligned(job.data.cameraPlane);
	job.data.cameraPlane[3] = -cameraPlane.distance();
	job.data.cullNearDistance = cullNearDistance;
	job.data.fadeNearRange = fadeNearRange;
	job.data.middleAge = middleAge;
	job.data.pointsEA = (uintptr_t)&points[0];
	job.data.pointsCount = size;
	job.data.vertexOutEA = (uintptr_t)(m_vertex + m_vertexOffset);
	job.data.batchEA = (uintptr_t)&batches.back();

	m_jobQueue->push(&job);

	m_vertexOffset += size * 4;

#else

	const float c_extents[4][2] =
	{
		{ -1.0f, -1.0f },
		{  1.0f, -1.0f },
		{  1.0f,  1.0f },
		{ -1.0f,  1.0f }
	};

	for (int32_t i = 0; i < size; ++i)
	{
		const Point& point = points[i];

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

		for (int j = 0; j < 4; ++j)
		{
			// \note We're assuming locked vertex buffer is 16-aligned.
			point.position.storeAligned(m_vertex->positionAndOrientation.position);
			point.velocity.storeAligned(m_vertex->velocityAndRandom.velocity);
			point.color.storeAligned(m_vertex->attrib2.color);

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

#endif
}

void PointRenderer::flush(
	render::RenderContext* renderContext,
	world::IWorldRenderPass& worldRenderPass
)
{
	if (m_vertexOffset > 0)
	{
#if defined(_PS3)
		m_jobQueue->wait();
#endif

		T_ASSERT (m_vertex);

		m_vertex = 0;
		m_vertexBuffers[m_count]->unlock();

		for (AlignedVector< Batch >::iterator i = m_batches.begin(); i != m_batches.end(); ++i)
		{
			if (!i->shader || !i->count)
				continue;

			worldRenderPass.setShaderTechnique(i->shader);
			worldRenderPass.setShaderCombination(i->shader);

			render::IProgram* program = i->shader->getCurrentProgram();
			if (!program)
				continue;

			render::IndexedRenderBlock* renderBlock = renderContext->alloc< render::IndexedRenderBlock >("PointRenderer");

			renderBlock->distance = i->distance;
			renderBlock->program = program;
			renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
			renderBlock->indexBuffer = m_indexBuffer;
			renderBlock->vertexBuffer = m_vertexBuffers[m_count];
			renderBlock->primitive = render::PtTriangles;
			renderBlock->offset = i->offset;
			renderBlock->count = i->count;
			renderBlock->minIndex = 0;
			renderBlock->maxIndex = m_vertexOffset;

			renderBlock->programParams->beginParameters(renderContext);
			worldRenderPass.setProgramParameters(renderBlock->programParams, i->shader->getCurrentPriority());
			renderBlock->programParams->endParameters(renderContext);

			renderContext->draw(i->shader->getCurrentPriority(), renderBlock);
		}

		m_vertexOffset = 0;
		m_count = (m_count + 1) % sizeof_array(m_vertexBuffers);
	}

	m_batches.resize(0);
}

	}
}
