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
		namespace
		{

#pragma pack(1)
struct Vertex
{
	float pos[2];
	uint8_t color[4];
};
#pragma pack()

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AccShapeVertexPool", AccShapeVertexPool, Object)

AccShapeVertexPool::AccShapeVertexPool(render::IRenderSystem* renderSystem)
:	m_renderSystem(renderSystem)
{
}

bool AccShapeVertexPool::create()
{
	const int32_t c_precreateBuffers[][2] =
	{
		{ 8, 16 },
		{ 32, 16 },
		{ 64, 8 },
		{ 128, 8 },
		{ 256, 8 },
		{ 512, 16 },
		{ 768, 16 },
		{ 1024, 8 }
	};

	std::vector< render::VertexElement > vertexElements(2);
	vertexElements[0] = render::VertexElement(render::DuPosition, render::DtFloat2, offsetof(Vertex, pos));
	vertexElements[1] = render::VertexElement(render::DuColor, render::DtByte4N, offsetof(Vertex, color), 0);
	T_ASSERT (render::getVertexSize(vertexElements) == sizeof(Vertex));

	for (int32_t i = 0; i < sizeof_array(c_precreateBuffers); ++i)
	{
		for (int32_t j = 0; j < c_precreateBuffers[i][1]; ++j)
		{
			Ref< render::VertexBuffer > vertexBuffer = m_renderSystem->createVertexBuffer(
				vertexElements,
				c_precreateBuffers[i][0] * 3 * sizeof(Vertex),
				false
			);
			if (!vertexBuffer)
			{
				log::error << L"Unable to create vertex buffer " << c_precreateBuffers[i][0] << Endl;
				return false;
			}

			PoolEntry pe;
			pe.vertexBuffer = vertexBuffer;
			pe.triangleCount = c_precreateBuffers[i][0];
			m_freeBuffers.push_back(pe);
		}
	}

	return true;
}

void AccShapeVertexPool::destroy()
{
	for (std::list< PoolEntry >::iterator i = m_freeBuffers.begin(); i != m_freeBuffers.end(); ++i)
		safeDestroy(i->vertexBuffer);
	for (std::list< PoolEntry >::iterator i = m_usedBuffers.begin(); i != m_usedBuffers.end(); ++i)
		safeDestroy(i->vertexBuffer);

	m_freeBuffers.clear();
	m_usedBuffers.clear();

	m_renderSystem = 0;
}

render::VertexBuffer* AccShapeVertexPool::acquireVertexBuffer(int32_t triangleCount)
{
	std::list< PoolEntry >::iterator min = m_freeBuffers.end();
	Ref< render::VertexBuffer > vertexBuffer;

	if (triangleCount <= 1024)
	{
		for (std::list< PoolEntry >::iterator i = m_freeBuffers.begin(); i != m_freeBuffers.end(); ++i)
		{
			int32_t wastedTriangles = i->triangleCount - triangleCount;
			if (wastedTriangles < 0)
				continue;

			if (wastedTriangles == 0)
			{
				min = i;
				break;
			}

			if (min != m_freeBuffers.end())
			{
				if (wastedTriangles < min->triangleCount - triangleCount)
					min = i;
			}
			else
				min = i;
		}
	}

	if (min != m_freeBuffers.end())
	{
		vertexBuffer = min->vertexBuffer;
		m_usedBuffers.push_back(*min);
		m_freeBuffers.erase(min);
	}
	else
	{
		std::vector< render::VertexElement > vertexElements(2);
		vertexElements[0] = render::VertexElement(render::DuPosition, render::DtFloat2, offsetof(Vertex, pos));
		vertexElements[1] = render::VertexElement(render::DuColor, render::DtByte4N, offsetof(Vertex, color), 0);
		T_ASSERT (render::getVertexSize(vertexElements) == sizeof(Vertex));

		triangleCount = alignUp(triangleCount, 32);

		vertexBuffer = m_renderSystem->createVertexBuffer(
			vertexElements,
			triangleCount * 3 * sizeof(Vertex),
			false
		);
		if (!vertexBuffer)
			return 0;

		PoolEntry pe;
		pe.vertexBuffer = vertexBuffer;
		pe.triangleCount = triangleCount;
		m_usedBuffers.push_back(pe);
	}

	return vertexBuffer;
}

void AccShapeVertexPool::releaseVertexBuffer(render::VertexBuffer* vertexBuffer)
{
	if (!vertexBuffer)
		return;
	for (std::list< PoolEntry >::iterator i = m_usedBuffers.begin(); i != m_usedBuffers.end(); ++i)
	{
		if (i->vertexBuffer == vertexBuffer)
		{
			m_freeBuffers.push_back(*i);
			m_usedBuffers.erase(i);
			return;
		}
	}
	T_FATAL_ERROR;
}

	}
}
