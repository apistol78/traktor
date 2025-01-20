/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Frame/RenderGraphTargetSetPool.h"

#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/Frame/RenderGraphTargetSet.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"

#include <cstring>

namespace traktor::render
{
namespace
{

const int32_t c_maxUnusuedFrames = 8;

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderGraphTargetSetPool", RenderGraphTargetSetPool, Object)

RenderGraphTargetSetPool::RenderGraphTargetSetPool(IRenderSystem* renderSystem)
	: m_renderSystem(renderSystem)
{
}

void RenderGraphTargetSetPool::destroy()
{
	m_renderSystem = nullptr;
	for (auto& pool : m_pool)
	{
		T_FATAL_ASSERT(pool.acquired.empty());
		for (auto& target : pool.free)
			safeDestroy(target.rts);
	}
	m_pool.clear();
}

RenderGraphTargetSet* RenderGraphTargetSetPool::acquire(
	const wchar_t* name,
	const RenderGraphTargetSetDesc& targetSetDesc,
	IRenderTargetSet* sharedDepthStencilTargetSet,
	bool sharedPrimaryDepthStencilTargetSet,
	int32_t referenceWidth,
	int32_t referenceHeight,
	uint32_t multiSample,
	bool doubleBuffered,
	uint32_t persistentHandle)
{
	// Create descriptor for given reference size.
	RenderTargetSetCreateDesc rtscd;
	rtscd.count = targetSetDesc.count;
	rtscd.width = referenceWidth;
	rtscd.height = referenceHeight;
	rtscd.multiSample = 0;
	rtscd.createDepthStencil = targetSetDesc.createDepthStencil;
	rtscd.usingPrimaryDepthStencil = sharedPrimaryDepthStencilTargetSet;
	rtscd.usingDepthStencilAsTexture = targetSetDesc.usingDepthStencilAsTexture;
	rtscd.usingDepthStencilAsStorage = false;
	rtscd.ignoreStencil = targetSetDesc.ignoreStencil || targetSetDesc.usingDepthStencilAsTexture; // Cannot have stencil on read-back depth targets.
	rtscd.generateMips = targetSetDesc.generateMips;

	if (rtscd.usingPrimaryDepthStencil)
		rtscd.multiSample = multiSample;

	for (int32_t i = 0; i < targetSetDesc.count; ++i)
	{
		rtscd.targets[i].format = targetSetDesc.targets[i].colorFormat;
		rtscd.targets[i].sRGB = targetSetDesc.targets[i].sRGB;
	}

	// Find pool matching target description.
	auto it = std::find_if(
		m_pool.begin(),
		m_pool.end(),
		[&](const RenderGraphTargetSetPool::Pool& p) {
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
			p.rtscd.usingDepthStencilAsStorage != rtscd.usingDepthStencilAsStorage ||
			p.rtscd.usingPrimaryDepthStencil != rtscd.usingPrimaryDepthStencil ||
			p.rtscd.ignoreStencil != rtscd.ignoreStencil ||
			p.rtscd.generateMips != rtscd.generateMips)
			return false;

		for (int32_t i = 0; i < p.rtscd.count; ++i)
			if (
				p.rtscd.targets[i].format != rtscd.targets[i].format ||
				p.rtscd.targets[i].sRGB != rtscd.targets[i].sRGB)
				return false;

		return true;
	});

	// Get or create pool.
	Pool* pool = nullptr;
	if (it != m_pool.end())
		pool = &(*it);
	else
	{
		pool = &m_pool.push_back();
		pool->rtscd = rtscd;
		pool->sharedDepthStencilTargetSet = sharedDepthStencilTargetSet;
		pool->persistentHandle = persistentHandle;
	}

	// Acquire free target, if no one left we need to create a new target.
	if (!pool->free.empty())
	{
		const Target target = pool->free.back();
		pool->free.pop_back();
		pool->acquired.push_back(target.rts);
		return target.rts;
	}
	else
	{
		if (sharedDepthStencilTargetSet)
		{
			const int32_t sharedWidth = sharedDepthStencilTargetSet->getWidth();
			const int32_t sharedHeight = sharedDepthStencilTargetSet->getHeight();
			T_FATAL_ASSERT(sharedWidth == rtscd.width);
			T_FATAL_ASSERT(sharedHeight == rtscd.height);
		}

		Ref< IRenderTargetSet > readTargetSet = m_renderSystem->createRenderTargetSet(rtscd, sharedDepthStencilTargetSet, name);

		Ref< IRenderTargetSet > writeTargetSet;
		if (!doubleBuffered)
			writeTargetSet = readTargetSet;
		else
			writeTargetSet = m_renderSystem->createRenderTargetSet(rtscd, sharedDepthStencilTargetSet, name);

		Ref< RenderGraphTargetSet > rts;
		if (readTargetSet && writeTargetSet)
		{
			rts = new RenderGraphTargetSet(readTargetSet, writeTargetSet);
			pool->acquired.push_back(rts);
		}
		else
			log::error << L"Failed to create render graph target set \"" << name << L"\" (" << rtscd.width << L"*" << rtscd.height << L")." << Endl;

		return rts;
	}
}

void RenderGraphTargetSetPool::release(Ref< RenderGraphTargetSet >& targetSet)
{
	T_ANONYMOUS_VAR(Ref< RenderGraphTargetSet >)(targetSet);
	for (auto& pool : m_pool)
	{
		auto it = std::remove_if(pool.acquired.begin(), pool.acquired.end(), [&](const RenderGraphTargetSet* rts) {
			return rts == targetSet;
		});
		if (it != pool.acquired.end())
		{
			pool.acquired.erase(it, pool.acquired.end());
			pool.free.push_back({ targetSet, 0 });
			break;
		}
	}
}

void RenderGraphTargetSetPool::cleanup()
{
	int32_t freed = 0;
	for (auto& pool : m_pool)
	{
		T_FATAL_ASSERT(pool.acquired.empty());
		auto it = std::remove_if(pool.free.begin(), pool.free.end(), [](const Target& target) {
			return target.unused > c_maxUnusuedFrames;
		});
		if (it != pool.free.end())
		{
			for (auto it2 = it; it2 != pool.free.end(); ++it2)
			{
				if (it2->rts)
					it2->rts->destroy();
				++freed;
			}
			pool.free.erase(it, pool.free.end());
		}
		for (auto& target : pool.free)
			target.unused++;
	}
	if (freed > 0)
	{
		auto it = std::remove_if(m_pool.begin(), m_pool.end(), [](const Pool& pool) {
			return pool.free.empty() && pool.acquired.empty();
		});
		m_pool.erase(it, m_pool.end());
	}
}

}
