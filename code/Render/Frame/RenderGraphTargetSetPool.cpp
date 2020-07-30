#include <cstring>
#include "Core/Log/Log.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/Frame/RenderGraphTargetSetPool.h"

namespace traktor
{
	namespace render
	{

RenderGraphTargetSetPool::RenderGraphTargetSetPool(IRenderSystem* renderSystem)
:	m_renderSystem(renderSystem)
{
}

IRenderTargetSet* RenderGraphTargetSetPool::acquire(
	const RenderGraphTargetSetDesc& targetSetDesc,
	IRenderTargetSet* sharedDepthStencilTargetSet,
	int32_t referenceWidth,
	int32_t referenceHeight,
	handle_t persistentHandle
)
{
	// Create descriptor for given reference size.
	RenderTargetSetCreateDesc rtscd = {};
	rtscd.count = targetSetDesc.count;
	rtscd.width = targetSetDesc.width;
	rtscd.height = targetSetDesc.height;
	rtscd.multiSample = 0;
	rtscd.createDepthStencil = targetSetDesc.createDepthStencil;
	rtscd.usingPrimaryDepthStencil = targetSetDesc.usingPrimaryDepthStencil;
	rtscd.usingDepthStencilAsTexture = targetSetDesc.usingDepthStencilAsTexture;
	rtscd.ignoreStencil = targetSetDesc.usingDepthStencilAsTexture;	// Cannot have stencil on read-back depth targets.
	rtscd.generateMips = targetSetDesc.generateMips;

	for (int32_t i = 0; i < targetSetDesc.count; ++i)
		rtscd.targets[i].format = targetSetDesc.targets[i].colorFormat;
		
	if (targetSetDesc.referenceWidthDenom > 0)
		rtscd.width = (referenceWidth + targetSetDesc.referenceWidthDenom - 1) / targetSetDesc.referenceWidthDenom;
	if (targetSetDesc.referenceHeightDenom > 0)
		rtscd.height = (referenceHeight + targetSetDesc.referenceHeightDenom - 1) / targetSetDesc.referenceHeightDenom;
	if (targetSetDesc.maxWidth > 0)
		rtscd.width = min< int32_t >(rtscd.width, targetSetDesc.maxWidth);
	if (targetSetDesc.maxHeight > 0)
		rtscd.height = min< int32_t >(rtscd.height, targetSetDesc.maxHeight);

	// Find pool matching target description.
	auto it = std::find_if(
        m_pool.begin(),
        m_pool.end(),
        [&](const RenderGraphTargetSetPool::Pool& p)
        {
            if (p.sharedDepthStencilTargetSet != sharedDepthStencilTargetSet)
                return false;

			if (p.persistentHandle != persistentHandle)
				return false;

            if (
				p.rtscd.count != rtscd.count ||
				p.rtscd.width != rtscd.width ||
				p.rtscd.height != rtscd.height ||
				p.rtscd.multiSample != rtscd.multiSample ||
				p.rtscd.createDepthStencil != rtscd.createDepthStencil ||
				p.rtscd.usingDepthStencilAsTexture != rtscd.usingDepthStencilAsTexture ||
				p.rtscd.usingPrimaryDepthStencil != rtscd.usingPrimaryDepthStencil ||
				p.rtscd.ignoreStencil != rtscd.ignoreStencil ||
				p.rtscd.generateMips != rtscd.generateMips
            )
                return false;

 			for (int32_t i = 0; i < p.rtscd.count; ++i)
			{
				if (
					p.rtscd.targets[i].format != rtscd.targets[i].format ||
					p.rtscd.targets[i].sRGB != rtscd.targets[i].sRGB
				)
					return false;
			}

            return true;
        }
    );

	// Get or create pool.
	Pool* pool = nullptr;
	if (it != m_pool.end())
		pool = &(*it);
	else
	{
		m_pool.resize(m_pool.size() + 1);
		pool = &m_pool.back();
		pool->rtscd = rtscd;
        pool->sharedDepthStencilTargetSet = sharedDepthStencilTargetSet;
		pool->persistentHandle = persistentHandle;
	}

	// Acquire free target, if no one left we need to create a new target.
	if (!pool->free.empty())
	{
		Ref< IRenderTargetSet > targetSet = pool->free.back();
		pool->free.pop_back();
		pool->acquired.push_back(targetSet);
		return targetSet;
	}
	else
	{
		if (sharedDepthStencilTargetSet)
		{
			int32_t sharedWidth = sharedDepthStencilTargetSet->getWidth();
			int32_t sharedHeight = sharedDepthStencilTargetSet->getHeight();
			T_FATAL_ASSERT(sharedWidth == rtscd.width);
			T_FATAL_ASSERT(sharedHeight == rtscd.height);
		}

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

void RenderGraphTargetSetPool::cleanup()
{
	//auto it = std::remove_if(m_pool.begin(), m_pool.end(), [](const Pool& pool) {
	//	return pool.acquired.empty();
	//});
	//m_pool.erase(it, m_pool.end());
}

	}
}
