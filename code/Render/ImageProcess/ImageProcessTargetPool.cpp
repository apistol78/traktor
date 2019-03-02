#include <cstring>
#include "Render/IRenderSystem.h"
#include "Render/RenderTargetSet.h"
#include "Render/ImageProcess/ImageProcessTargetPool.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct FindPoolPred
{
	const RenderTargetSetCreateDesc& m_rtscd;

	FindPoolPred(const RenderTargetSetCreateDesc& rtscd)
	:	m_rtscd(rtscd)
	{
	}

	bool operator () (const ImageProcessTargetPool::Pool& p) const
	{
		if (
			p.rtscd.count != m_rtscd.count ||
			p.rtscd.width != m_rtscd.width ||
			p.rtscd.height != m_rtscd.height ||
			p.rtscd.multiSample != m_rtscd.multiSample ||
			p.rtscd.createDepthStencil != m_rtscd.createDepthStencil ||
			p.rtscd.usingDepthStencilAsTexture != m_rtscd.usingDepthStencilAsTexture ||
			p.rtscd.usingPrimaryDepthStencil != m_rtscd.usingPrimaryDepthStencil ||
			p.rtscd.preferTiled != m_rtscd.preferTiled ||
			p.rtscd.ignoreStencil != m_rtscd.ignoreStencil ||
			p.rtscd.generateMips != m_rtscd.generateMips
		)
			return false;

		for (int32_t i = 0; i < p.rtscd.count; ++i)
		{
			if (
				p.rtscd.targets[i].format != m_rtscd.targets[i].format ||
				p.rtscd.targets[i].sRGB != m_rtscd.targets[i].sRGB
			)
				return false;
		}

		return true;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImageProcessTargetPool", ImageProcessTargetPool, Object)

ImageProcessTargetPool::ImageProcessTargetPool(IRenderSystem* renderSystem)
:	m_renderSystem(renderSystem)
{
}

RenderTargetSet* ImageProcessTargetPool::acquireTarget(const RenderTargetSetCreateDesc& rtscd)
{
	Pool* pool = 0;

	// Find or add new pool.
	AlignedVector< Pool >::iterator it = std::find_if(m_pool.begin(), m_pool.end(), FindPoolPred(rtscd));
	if (it != m_pool.end())
		pool = &(*it);
	else
	{
		m_pool.resize(m_pool.size() + 1);
		pool = &m_pool.back();
		pool->rtscd = rtscd;
	}

	if (!pool->free.empty())
	{
		Ref< RenderTargetSet > rts = pool->free.back();
		pool->free.pop_back();
		pool->acquired.push_back(rts);
		return rts;
	}
	else
	{
		Ref< RenderTargetSet > rts = m_renderSystem->createRenderTargetSet(rtscd);
		if (rts)
			pool->acquired.push_back(rts);
		return rts;
	}
}

void ImageProcessTargetPool::releaseTarget(const RenderTargetSetCreateDesc& rtscd, RenderTargetSet* rts)
{
	AlignedVector< Pool >::iterator it = std::find_if(m_pool.begin(), m_pool.end(), FindPoolPred(rtscd));
	T_ASSERT (it != m_pool.end());

	it->acquired.remove(rts);
	it->free.push_back(rts);

	rts->discard();
}

	}
}
