#include <cstring>
#include "Core/Misc/Adler32.h"
#include "Render/IRenderSystem.h"
#include "Render/RenderTargetSet.h"
#include "World/PostProcess/PostProcessTargetPool.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

uint32_t hash(const render::RenderTargetSetCreateDesc& rtscd)
{
	Adler32 a;
	a.begin();
	a.feed(&rtscd, sizeof(render::RenderTargetSetCreateDesc));
	a.end();
	return a.get();
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PostProcessTargetPool", PostProcessTargetPool, Object)

PostProcessTargetPool::PostProcessTargetPool(render::IRenderSystem* renderSystem)
:	m_renderSystem(renderSystem)
{
}

render::RenderTargetSet* PostProcessTargetPool::acquireTarget(const render::RenderTargetSetCreateDesc& rtscd)
{
	Pool& pool = m_pool[hash(rtscd)];
	if (!pool.free.empty())
	{
		T_ASSERT (std::memcmp(&rtscd, &pool.rtscd, sizeof(rtscd)) == 0);

		Ref< render::RenderTargetSet > rts = pool.free.back();
		pool.free.pop_back();

		pool.acquired.push_back(rts);
		return rts;
	}
	else
	{
		pool.rtscd = rtscd;

		Ref< render::RenderTargetSet > rts = m_renderSystem->createRenderTargetSet(rtscd);
		if (!rts)
			return 0;

		pool.acquired.push_back(rts);
		return rts;
	}
}

void PostProcessTargetPool::releaseTarget(const render::RenderTargetSetCreateDesc& rtscd, render::RenderTargetSet* rts)
{
	Pool& pool = m_pool[hash(rtscd)];
	T_ASSERT (std::memcmp(&rtscd, &pool.rtscd, sizeof(rtscd)) == 0);

	pool.acquired.remove(rts);
	pool.free.push_back(rts);

	rts->discard();
}

	}
}
