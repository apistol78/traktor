#include <algorithm>
#include "Core/Memory/Alloc.h"
#include "Render/Context/RenderContext.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

T_FORCE_INLINE bool SortOpaquePredicate(const RenderBlock* renderBlock1, const RenderBlock* renderBlock2)
{
	const float c_distanceQuantizeRangeInv = 1.0f / 4.0f;
	float d1 = std::floor(renderBlock1->distance * c_distanceQuantizeRangeInv);
	float d2 = std::floor(renderBlock2->distance * c_distanceQuantizeRangeInv);

	if (d1 < d2)
		return true;
	else if (d1 > d2)
		return false;

	return renderBlock1->program < renderBlock2->program;
}

T_FORCE_INLINE bool SortAlphaBlendPredicate(const RenderBlock* renderBlock1, const RenderBlock* renderBlock2)
{
	return renderBlock1->distance > renderBlock2->distance;
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
		m_renderQueue[0].push_back(renderBlock);
	else if (type == RfAlphaBlend)
		m_renderQueue[1].push_back(renderBlock);
	else	// RbtOverlay
		m_renderQueue[2].push_back(renderBlock);
}

void RenderContext::render(render::IRenderView* renderView, uint32_t flags, const ProgramParameters* globalParameters) const
{
	// Render opaque blocks, sorted by shader.
	if (flags & RfOpaque)
	{
		std::sort(m_renderQueue[0].begin(), m_renderQueue[0].end(), SortOpaquePredicate);
		for (std::vector< RenderBlock* >::const_iterator i = m_renderQueue[0].begin(); i != m_renderQueue[0].end(); ++i)
			(*i)->render(renderView, globalParameters);
	}

	// Render alpha blend blocks back to front.
	if (flags & RfAlphaBlend)
	{
		std::sort(m_renderQueue[1].begin(), m_renderQueue[1].end(), SortAlphaBlendPredicate);
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
