#include "Core/Memory/Alloc.h"
#include "Render/Context/RenderContext.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct SortOpaquePredicate
{
	T_FORCE_INLINE bool operator () (const RenderBlock* renderBlock1, const RenderBlock* renderBlock2) const
	{
		if (renderBlock1->program < renderBlock2->program)
			return true;
		if (renderBlock1->program > renderBlock2->program)
			return false;
		return renderBlock1->distance < renderBlock2->distance;
	}
};

struct SortAlphaBlendPredicate
{
	T_FORCE_INLINE bool operator () (const RenderBlock* renderBlock1, const RenderBlock* renderBlock2) const
	{
		return renderBlock1->distance > renderBlock2->distance;
	}
};

template < typename Predicate >
T_FORCE_INLINE void insert(std::vector< RenderBlock* >& queue, RenderBlock* renderBlock, Predicate predicate)
{
	int32_t a = 0;
	int32_t b = int32_t(queue.size());

	while (a < b)
	{
		int32_t i = (a + b) >> 1;
		if (predicate(queue[i], renderBlock))
			a = i + 1;
		else
			b = i;
	}

	queue.insert(queue.begin() + a, renderBlock);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderContext", RenderContext, Object)

RenderContext::RenderContext(uint32_t heapSize)
:	m_heap(0)
,	m_heapEnd(0)
,	m_heapPtr(0)
{
	m_heap = static_cast< uint8_t* >(Alloc::acquireAlign(heapSize, 16, T_FILE_LINE));
	T_FATAL_ASSERT_M (m_heap, L"Out of memory (Render context)");
	m_heapEnd = m_heap + heapSize;
	m_heapPtr = m_heap;
}

RenderContext::~RenderContext()
{
	Alloc::freeAlign(m_heap);
}

void* RenderContext::alloc(int blockSize)
{
	if (m_heapPtr + blockSize >= m_heapEnd)
		T_FATAL_ERROR;

	void* ptr = m_heapPtr;
	m_heapPtr += blockSize;

	return ptr;
}

void* RenderContext::alloc(int blockSize, int align)
{
	T_ASSERT (align > 0);
	m_heapPtr = reinterpret_cast< uint8_t* >((size_t(m_heapPtr) + (align - 1)) & ~(align - 1));
	return alloc(blockSize);
}

void RenderContext::draw(uint32_t type, RenderBlock* renderBlock)
{
	if (type == RfOpaque)
		insert(m_renderQueue[0], renderBlock, SortOpaquePredicate());
	else if (type == RfAlphaBlend)
		insert(m_renderQueue[1], renderBlock, SortAlphaBlendPredicate());
	else	// RbtOverlay
		m_renderQueue[2].push_back(renderBlock);
}

void RenderContext::render(render::IRenderView* renderView, uint32_t flags, const ProgramParameters* globalParameters) const
{
	// Render opaque blocks, sorted by shader.
	if (flags & RfOpaque)
	{
		for (std::vector< RenderBlock* >::const_iterator i = m_renderQueue[0].begin(); i != m_renderQueue[0].end(); ++i)
			(*i)->render(renderView, globalParameters);
	}

	// Render alpha blend blocks back to front.
	if (flags & RfAlphaBlend)
	{
		for (std::vector< RenderBlock* >::const_iterator i = m_renderQueue[1].begin(); i != m_renderQueue[1].end(); ++i)
			(*i)->render(renderView, globalParameters);
	}

	// Render overlay blocks unsorted.
	if (flags & RfOverlay)
	{
		for (std::vector< RenderBlock* >::const_iterator i = m_renderQueue[2].begin(); i != m_renderQueue[2].end(); ++i)
			(*i)->render(renderView, globalParameters);
	}
}

void RenderContext::flush()
{
	// Reset queues and heap.
	for (int i = 0; i < sizeof_array(m_renderQueue); ++i)
		m_renderQueue[i].resize(0);

	m_heapPtr = m_heap;
}

	}
}
