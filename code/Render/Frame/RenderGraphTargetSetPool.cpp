#include <cstring>
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/Frame/RenderGraphTargetSetPool.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderGraphTargetSetPool", RenderGraphTargetSetPool, Object)

RenderGraphTargetSetPool::RenderGraphTargetSetPool(IRenderSystem* renderSystem, int32_t width, int32_t height)
:	m_renderSystem(renderSystem)
,   m_width(width)
,   m_height(height)
{
}

IRenderTargetSet* RenderGraphTargetSetPool::acquire(const RenderGraphTargetSetDesc& targetSetDesc, IRenderTargetSet* sharedDepthStencilTargetSet)
{
	Pool* pool = nullptr;

	auto it = std::find_if(
        m_pool.begin(),
        m_pool.end(),
        [&](const RenderGraphTargetSetPool::Pool& p)
        {
            if (p.sharedDepthStencilTargetSet != sharedDepthStencilTargetSet)
                return false;

            if (
                p.targetSetDesc.count != targetSetDesc.count ||
                p.targetSetDesc.width != targetSetDesc.width ||
                p.targetSetDesc.height != targetSetDesc.height ||
                p.targetSetDesc.screenWidthDenom != targetSetDesc.screenWidthDenom ||
                p.targetSetDesc.screenHeightDenom != targetSetDesc.screenHeightDenom ||
                p.targetSetDesc.maxWidth != targetSetDesc.maxWidth ||
                p.targetSetDesc.maxHeight != targetSetDesc.maxHeight ||
                p.targetSetDesc.createDepthStencil != targetSetDesc.createDepthStencil ||
                p.targetSetDesc.usingPrimaryDepthStencil != targetSetDesc.usingPrimaryDepthStencil ||
                p.targetSetDesc.usingDepthStencilAsTexture != targetSetDesc.usingDepthStencilAsTexture ||
                p.targetSetDesc.ignoreStencil != targetSetDesc.ignoreStencil ||
                p.targetSetDesc.generateMips != targetSetDesc.generateMips
            )
                return false;

            for (int32_t i = 0; i < p.targetSetDesc.count; ++i)
            {
                if (p.targetSetDesc.targets[i].colorFormat != targetSetDesc.targets[i].colorFormat)
                    return false;
            }

            return true;
        }
    );
	if (it != m_pool.end())
		pool = &(*it);
	else
	{
		m_pool.resize(m_pool.size() + 1);
		pool = &m_pool.back();
		pool->targetSetDesc = targetSetDesc;
        pool->sharedDepthStencilTargetSet = sharedDepthStencilTargetSet;
	}

	if (!pool->free.empty())
	{
		Ref< IRenderTargetSet > targetSet = pool->free.back();
		pool->free.pop_back();
		pool->acquired.push_back(targetSet);
		return targetSet;
	}
	else
	{
		RenderTargetSetCreateDesc rtscd = {};
		rtscd.count = targetSetDesc.count;
		rtscd.width = targetSetDesc.width;
		rtscd.height = targetSetDesc.height;
		rtscd.multiSample = 0;
		rtscd.createDepthStencil = targetSetDesc.createDepthStencil;
		rtscd.usingPrimaryDepthStencil = targetSetDesc.usingPrimaryDepthStencil;
		rtscd.usingDepthStencilAsTexture = targetSetDesc.usingDepthStencilAsTexture;
		rtscd.storeDepthStencil = true;
		rtscd.ignoreStencil = false;
		rtscd.generateMips = targetSetDesc.generateMips;

		for (int32_t i = 0; i < targetSetDesc.count; ++i)
			rtscd.targets[i].format = targetSetDesc.targets[i].colorFormat;
		
		if (targetSetDesc.screenWidthDenom > 0)
			rtscd.width = (m_width + targetSetDesc.screenWidthDenom - 1) / targetSetDesc.screenWidthDenom;
		if (targetSetDesc.screenHeightDenom > 0)
			rtscd.height = (m_height + targetSetDesc.screenHeightDenom - 1) / targetSetDesc.screenHeightDenom;
		if (targetSetDesc.maxWidth > 0)
			rtscd.width = min< int32_t >(rtscd.width, targetSetDesc.maxWidth);
		if (targetSetDesc.maxHeight > 0)
			rtscd.height = min< int32_t >(rtscd.height, targetSetDesc.maxHeight);

		Ref< IRenderTargetSet > targetSet = m_renderSystem->createRenderTargetSet(rtscd, sharedDepthStencilTargetSet, T_FILE_LINE_W);
		if (targetSet)
			pool->acquired.push_back(targetSet);

		return targetSet;
	}
}

void RenderGraphTargetSetPool::release(IRenderTargetSet* targetSet)
{
	T_ANONYMOUS_VAR(Ref< IRenderTargetSet >)(targetSet);
	for (auto it = m_pool.begin(); it != m_pool.end(); ++it)
	{
		if (it->acquired.remove(targetSet))
		{
			it->free.push_back(targetSet);
			break;
		}
	}
}

	}
}
