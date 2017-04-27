/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/SafeDestroy.h"
#include "Flash/Acc/AccShapeVertexPool.h"
#include "Render/IRenderSystem.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AccShapeVertexPool", AccShapeVertexPool, Object)

AccShapeVertexPool::AccShapeVertexPool(render::IRenderSystem* renderSystem, uint32_t frameCount)
:	m_renderSystem(renderSystem)
,	m_frame(0)
{
	m_garbageRanges.resize(frameCount);
}

bool AccShapeVertexPool::create()
{
	return true;
}

void AccShapeVertexPool::destroy()
{
	for (uint32_t i = 0; i < uint32_t(m_garbageRanges.size()); ++i)
		cycleGarbage();

	for (std::list< VertexRange >::iterator i = m_freeRanges.begin(); i != m_freeRanges.end(); ++i)
		safeDestroy(i->vertexBuffer);

	for (std::list< VertexRange >::iterator i = m_usedRanges.begin(); i != m_usedRanges.end(); ++i)
		safeDestroy(i->vertexBuffer);

	m_freeRanges.clear();
	m_usedRanges.clear();
	
	m_renderSystem = 0;
}

bool AccShapeVertexPool::acquireRange(int32_t vertexCount, Range& outRange)
{
	for (std::list< VertexRange >::iterator i = m_freeRanges.begin(); i != m_freeRanges.end(); ++i)
	{
		if (i->vertexCount >= vertexCount)
		{
			outRange.vertexBuffer = i->vertexBuffer;

			m_usedRanges.push_back(*i);
			m_freeRanges.erase(i);

			return true;
		}
	}

	std::vector< render::VertexElement > vertexElements(5);
	vertexElements[0] = render::VertexElement(render::DuPosition, render::DtFloat2, offsetof(Vertex, pos));
	vertexElements[1] = render::VertexElement(render::DuCustom, render::DtByte4N, offsetof(Vertex, curvature), 0);
	vertexElements[2] = render::VertexElement(render::DuCustom, render::DtFloat2, offsetof(Vertex, texCoord), 1);
	vertexElements[3] = render::VertexElement(render::DuCustom, render::DtFloat4, offsetof(Vertex, texRect), 2);
	vertexElements[4] = render::VertexElement(render::DuColor, render::DtByte4N, offsetof(Vertex, color), 0);
	T_FATAL_ASSERT (render::getVertexSize(vertexElements) == sizeof(Vertex));

	Ref< render::VertexBuffer > vertexBuffer = m_renderSystem->createVertexBuffer(
		vertexElements,
		vertexCount * sizeof(Vertex),
		false
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

	for (std::list< VertexRange >::iterator i = m_usedRanges.begin(); i != m_usedRanges.end(); ++i)
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
	m_frame = (m_frame + 1) % uint32_t(m_garbageRanges.size());
	m_freeRanges.insert(m_freeRanges.end(), m_garbageRanges[m_frame].begin(), m_garbageRanges[m_frame].end());
	m_garbageRanges[m_frame].clear();
}

	}
}
