#include <algorithm>
#include "Core/Memory/Alloc.h"
#include "Render/Context/RenderContext.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const float c_distanceQuantizeRangeInv = 1.0f / 10.0f;

T_FORCE_INLINE bool SortOpaquePredicate(const RenderBlock* renderBlock1, const RenderBlock* renderBlock2)
{
// Don't sort front-to-back on iOS as it's a TDBR architecture thus
// we focus on minimizing state changes on the CPU instead.
#if !defined(__IOS__)
	float d1 = std::floor(renderBlock1->distance * c_distanceQuantizeRangeInv);
	float d2 = std::floor(renderBlock2->distance * c_distanceQuantizeRangeInv);

	if (d1 < d2)
		return true;
	else if (d1 > d2)
		return false;
#endif

	return renderBlock1->program < renderBlock2->program;
}

T_FORCE_INLINE bool SortAlphaBlendPredicate(const RenderBlock* renderBlock1, const RenderBlock* renderBlock2)
{
	return renderBlock1->distance > renderBlock2->distance;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderContext", RenderContext, Object)

RenderContext::RenderContext(uint32_t heapSize)
:	m_heapEnd(0)
,	m_heapPtr(0)
{
	m_heap.reset(static_cast< uint8_t* >(Alloc::acquireAlign(heapSize, 16, T_FILE_LINE)));
	T_FATAL_ASSERT_M (m_heap.ptr(), L"Out of memory (Render context)");
	m_heapEnd = m_heap.ptr() + heapSize;
	m_heapPtr = m_heap.ptr();
}

RenderContext::~RenderContext()
{
	m_heap.release();
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
	if (type == RpSetup)
		m_renderQueue[0].push_back(renderBlock);
	else if (type == RpOpaque)
		m_renderQueue[1].push_back(renderBlock);
	else if (type == RpPostOpaque)
		m_renderQueue[2].push_back(renderBlock);
	else if (type == RpAlphaBlend)
		m_renderQueue[3].push_back(renderBlock);
	else if (type == RpPostAlphaBlend)
		m_renderQueue[4].push_back(renderBlock);
	else if (type == RpOverlay)
		m_renderQueue[5].push_back(renderBlock);
}

void RenderContext::render(IRenderView* renderView, uint32_t priorities, const ProgramParameters* globalParameters) const
{
	// Render setup blocks unsorted.
	if (priorities & RpSetup)
	{
		for (AlignedVector< RenderBlock* >::const_iterator i = m_renderQueue[0].begin(); i != m_renderQueue[0].end(); ++i)
			(*i)->render(renderView, globalParameters);
	}

	// Render opaque blocks, sorted by shader.
	if (priorities & RpOpaque)
	{
		std::sort(m_renderQueue[1].begin(), m_renderQueue[1].end(), SortOpaquePredicate);
		for (AlignedVector< RenderBlock* >::const_iterator i = m_renderQueue[1].begin(); i != m_renderQueue[1].end(); ++i)
			(*i)->render(renderView, globalParameters);
	}

	// Render post opaque blocks, sorted by shader.
	if (priorities & RpPostOpaque)
	{
		std::sort(m_renderQueue[2].begin(), m_renderQueue[2].end(), SortOpaquePredicate);
		for (AlignedVector< RenderBlock* >::const_iterator i = m_renderQueue[2].begin(); i != m_renderQueue[2].end(); ++i)
			(*i)->render(renderView, globalParameters);
	}

	// Render alpha blend blocks back to front.
	if (priorities & RpAlphaBlend)
	{
		std::sort(m_renderQueue[3].begin(), m_renderQueue[3].end(), SortAlphaBlendPredicate);
		for (AlignedVector< RenderBlock* >::const_iterator i = m_renderQueue[3].begin(); i != m_renderQueue[3].end(); ++i)
			(*i)->render(renderView, globalParameters);
	}

	// Render post alpha blend blocks back to front.
	if (priorities & RpPostAlphaBlend)
	{
		std::sort(m_renderQueue[4].begin(), m_renderQueue[4].end(), SortAlphaBlendPredicate);
		for (AlignedVector< RenderBlock* >::const_iterator i = m_renderQueue[4].begin(); i != m_renderQueue[4].end(); ++i)
			(*i)->render(renderView, globalParameters);
	}

	// Render overlay blocks unsorted.
	if (priorities & RpOverlay)
	{
		for (AlignedVector< RenderBlock* >::const_iterator i = m_renderQueue[5].begin(); i != m_renderQueue[5].end(); ++i)
			(*i)->render(renderView, globalParameters);
	}
}

void RenderContext::flush()
{
	// Reset queues and heap.
	for (int32_t i = 0; i < sizeof_array(m_renderQueue); ++i)
	{
		// As blocks are allocated from a fixed pool we need to manually call destructors.
		for (AlignedVector< RenderBlock* >::const_iterator j = m_renderQueue[i].begin(); j != m_renderQueue[i].end(); ++j)
			(*j)->~RenderBlock();

		m_renderQueue[i].resize(0);
	}

	m_heapPtr = m_heap.ptr();
}

	}
}
