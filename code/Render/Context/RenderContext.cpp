#include "Render/Context/RenderContext.h"
#include "Core/Heap/Alloc.h"

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
		T_ASSERT (renderBlock1->type == RbtOpaque);
		if (renderBlock1->shader < renderBlock2->shader)
			return true;
		if (renderBlock1->shader > renderBlock2->shader)
			return false;
		return renderBlock1->distance < renderBlock2->distance;
	}
};

struct SortAlphaBlendPredicate
{
	T_FORCE_INLINE bool operator () (const RenderBlock* renderBlock1, const RenderBlock* renderBlock2) const
	{
		T_ASSERT (renderBlock1->type == RbtAlphaBlend);
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

RenderContext::RenderContext(RenderView* renderView, uint32_t heapSize)
:	m_renderView(renderView)
,	m_heap(0)
,	m_heapEnd(0)
,	m_heapPtr(0)
{
	m_heap = static_cast< uint8_t* >(allocAlign(heapSize, 16));
	m_heapEnd = m_heap + heapSize;
	m_heapPtr = m_heap;
}

RenderContext::~RenderContext()
{
	freeAlign(m_heap);
}

void* RenderContext::alloc(int blockSize)
{
	if (m_heapPtr + blockSize >= m_heapEnd)
		return 0;

	void* ptr = m_heapPtr;
	m_heapPtr += blockSize;

	return ptr;
}

void RenderContext::draw(RenderBlock* renderBlock)
{
	if (renderBlock->type == RbtOpaque)
		insert(m_renderQueue[RbtOpaque], renderBlock, SortOpaquePredicate());
	else
		insert(m_renderQueue[RbtAlphaBlend], renderBlock, SortAlphaBlendPredicate());
}

void RenderContext::render(uint32_t flags) const
{
	// Render opaque blocks, sorted by shader.
	if (flags & RfOpaque)
	{
		for (std::vector< RenderBlock* >::const_iterator i = m_renderQueue[RbtOpaque].begin(); i != m_renderQueue[RbtOpaque].end(); ++i)
			(*i)->render(m_renderView);
	}

	// Render alpha blend blocks back to front.
	if (flags & RfAlphaBlend)
	{
		for (std::vector< RenderBlock* >::const_iterator i = m_renderQueue[RbtAlphaBlend].begin(); i != m_renderQueue[RbtAlphaBlend].end(); ++i)
			(*i)->render(m_renderView);
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
