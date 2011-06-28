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

const uint32_t c_vertexPoolSize = 65536;

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
	std::vector< render::VertexElement > vertexElements(2);
	vertexElements[0] = render::VertexElement(render::DuPosition, render::DtFloat2, offsetof(Vertex, pos));
	vertexElements[1] = render::VertexElement(render::DuColor, render::DtByte4N, offsetof(Vertex, color), 0);
	T_ASSERT (render::getVertexSize(vertexElements) == sizeof(Vertex));

	Ref< render::VertexBuffer > vertexBuffer = m_renderSystem->createVertexBuffer(
		vertexElements,
		c_vertexPoolSize * sizeof(Vertex),
		false
	);
	if (!vertexBuffer)
		return false;

	void* ptr = vertexBuffer->lock();
	if (!ptr)
		return false;
		
	std::memset(ptr, 0, c_vertexPoolSize * sizeof(Vertex));
	vertexBuffer->unlock();

	VertexPool pool;
	pool.vertexBuffer = vertexBuffer;
	pool.blockList = new BlockList(c_vertexPoolSize);
	m_pools.push_back(pool);

	return true;
}

void AccShapeVertexPool::destroy()
{
	for (std::list< VertexPool >::iterator i = m_pools.begin(); i != m_pools.end(); ++i)
	{
		safeDestroy(i->vertexBuffer);
		delete i->blockList;
	}

	m_pools.clear();
	m_renderSystem = 0;
}

bool AccShapeVertexPool::acquireRange(int32_t vertexCount, Range& outRange)
{
	if (vertexCount > c_vertexPoolSize)
		return false;

	for (std::list< VertexPool >::iterator i = m_pools.begin(); i != m_pools.end(); ++i)
	{
		uint32_t offset = i->blockList->alloc(vertexCount, 1);
		if (offset != BlockList::NotEnoughSpace)
		{
			outRange.vertexBuffer = i->vertexBuffer;
			outRange.offset = offset;
			return true;
		}
	}

	log::debug << L"Out of memory in vertex pools; allocating another buffer" << Endl;

	std::vector< render::VertexElement > vertexElements(2);
	vertexElements[0] = render::VertexElement(render::DuPosition, render::DtFloat2, offsetof(Vertex, pos));
	vertexElements[1] = render::VertexElement(render::DuColor, render::DtByte4N, offsetof(Vertex, color), 0);
	T_ASSERT (render::getVertexSize(vertexElements) == sizeof(Vertex));

	Ref< render::VertexBuffer > vertexBuffer = m_renderSystem->createVertexBuffer(
		vertexElements,
		c_vertexPoolSize * sizeof(Vertex),
		false
	);
	if (!vertexBuffer)
		return false;

	void* ptr = vertexBuffer->lock();
	if (!ptr)
		return false;
		
	std::memset(ptr, 0, c_vertexPoolSize * sizeof(Vertex));
	vertexBuffer->unlock();

	VertexPool pool;
	pool.vertexBuffer = vertexBuffer;
	pool.blockList = new BlockList(c_vertexPoolSize);
	m_pools.push_back(pool);

	outRange.vertexBuffer = pool.vertexBuffer;
	outRange.offset = pool.blockList->alloc(vertexCount, 1);
	T_ASSERT (outRange.offset != BlockList::NotEnoughSpace);

	return true;
}

void AccShapeVertexPool::releaseRange(const Range& range)
{
	if (!range.vertexBuffer)
		return;

	for (std::list< VertexPool >::iterator i = m_pools.begin(); i != m_pools.end(); ++i)
	{
		if (i->vertexBuffer == range.vertexBuffer)
		{
			i->blockList->free(range.offset);
			return;
		}
	}

	T_FATAL_ERROR;
}

	}
}
