#include "Core/Log/Log.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/SafeDestroy.h"
#include "Spark/Acc/AccShapeVertexPool.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.AccShapeVertexPool", AccShapeVertexPool, Object)

AccShapeVertexPool::AccShapeVertexPool(render::IRenderSystem* renderSystem, uint32_t frameCount)
:	m_renderSystem(renderSystem)
{
	m_garbageRanges.resize(frameCount);
}

bool AccShapeVertexPool::create(const AlignedVector< render::VertexElement >& vertexElements)
{
	m_vertexLayout = m_renderSystem->createVertexLayout(vertexElements);
	m_vertexSize = render::getVertexSize(vertexElements);
	return true;
}

void AccShapeVertexPool::destroy()
{
	for (uint32_t i = 0; i < (uint32_t)m_garbageRanges.size(); ++i)
		cycleGarbage();

	for (auto& freeRange : m_freeRanges)
		safeDestroy(freeRange.vertexBuffer);

	for (auto& usedRange : m_usedRanges)
		safeDestroy(usedRange.vertexBuffer);

	m_freeRanges.clear();
	m_usedRanges.clear();

	m_renderSystem = nullptr;
}

bool AccShapeVertexPool::acquireRange(int32_t vertexCount, Range& outRange)
{
	for (auto i = m_freeRanges.begin(); i != m_freeRanges.end(); ++i)
	{
		if (i->vertexCount >= vertexCount)
		{
			outRange.vertexBuffer = i->vertexBuffer;
			m_usedRanges.push_back(*i);
			m_freeRanges.erase(i);
			return true;
		}
	}

	Ref< render::Buffer > vertexBuffer = m_renderSystem->createBuffer(
		render::BuVertex,
		vertexCount * m_vertexSize,
		true
	);
	if (!vertexBuffer)
		return false;

	VertexRange range;
	range.vertexBuffer = vertexBuffer;
	range.vertexCount = vertexCount;
	m_usedRanges.push_back(range);

	outRange.vertexBuffer = vertexBuffer;
	return true;
}

void AccShapeVertexPool::releaseRange(const Range& range)
{
	if (!range.vertexBuffer)
		return;

	for (auto i = m_usedRanges.begin(); i != m_usedRanges.end(); ++i)
	{
		if (i->vertexBuffer == range.vertexBuffer)
		{
			m_garbageRanges[m_frame].push_back(*i);
			m_usedRanges.erase(i);
			return;
		}
	}

	T_FATAL_ERROR;
}

void AccShapeVertexPool::cycleGarbage()
{
	m_frame = (m_frame + 1) % (uint32_t)m_garbageRanges.size();
	m_freeRanges.insert(m_freeRanges.end(), m_garbageRanges[m_frame].begin(), m_garbageRanges[m_frame].end());
	m_garbageRanges[m_frame].clear();
}

	}
}
