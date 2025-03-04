/*
 * TRAKTOR
 * Copyright (c) 2023-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Frame/RenderGraphTexturePool.h"

#include "Core/Log/Log.h"
#include "Render/IRenderSystem.h"
#include "Render/ITexture.h"

#include <cstring>

namespace traktor::render
{
namespace
{

const int32_t c_maxUnusuedFrames = 8;

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderGraphTexturePool", RenderGraphTexturePool, Object)

RenderGraphTexturePool::RenderGraphTexturePool(IRenderSystem* renderSystem)
	: m_renderSystem(renderSystem)
{
}

void RenderGraphTexturePool::destroy()
{
	m_renderSystem = nullptr;
}

Ref< ITexture > RenderGraphTexturePool::acquire(const RenderGraphTextureDesc& textureDesc, int32_t referenceWidth, int32_t referenceHeight, uint32_t persistentHandle)
{
	// Calculate required size of buffer.
	int32_t width = textureDesc.width, height = textureDesc.height;
	if (textureDesc.referenceWidthDenom > 0)
		width += (referenceWidth * textureDesc.referenceWidthMul + textureDesc.referenceWidthDenom - 1) / textureDesc.referenceWidthDenom;
	if (textureDesc.referenceHeightDenom > 0)
		height += (referenceHeight * textureDesc.referenceHeightMul + textureDesc.referenceHeightDenom - 1) / textureDesc.referenceHeightDenom;

	auto it = std::find_if(
		m_pool.begin(),
		m_pool.end(),
		[&](const RenderGraphTexturePool::Pool& p) {
		if (p.persistentHandle != persistentHandle)
			return false;

		if (p.textureDesc.width != width || p.textureDesc.height != height || p.textureDesc.mipCount != textureDesc.mipCount || p.textureDesc.format != textureDesc.format)
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
		pool->textureDesc = textureDesc;
		pool->textureDesc.width = width;
		pool->textureDesc.height = height;
		pool->persistentHandle = persistentHandle;
	}

	// Acquire free texture, if no one left we need to create a new texture.
	if (!pool->free.empty())
	{
		Ref< ITexture > texture = pool->free.back().texture;

		pool->free.pop_back();
		pool->acquired.push_back(texture);

		return texture;
	}
	else
	{
		SimpleTextureCreateDesc stcd;
		stcd.width = width;
		stcd.height = height;
		stcd.mipCount = textureDesc.mipCount;
		stcd.format = textureDesc.format;
		stcd.shaderStorage = true;

		Ref< ITexture > texture = m_renderSystem->createSimpleTexture(stcd, T_FILE_LINE_W);
		if (texture)
			pool->acquired.push_back(texture);

		T_DEBUG(L"Allocated RG texture");
		return texture;
	}
}

void RenderGraphTexturePool::release(Ref< ITexture >& texture)
{
	T_ANONYMOUS_VAR(Ref< ITexture >)(texture);
	for (auto& pool : m_pool)
	{
		auto it = std::remove_if(pool.acquired.begin(), pool.acquired.end(), [&](const ITexture* tx) {
			return tx == texture;
		});
		if (it != pool.acquired.end())
		{
			pool.acquired.erase(it, pool.acquired.end());
			pool.free.push_back({ texture, 0 });
			break;
		}
	}
	texture = nullptr;
}

void RenderGraphTexturePool::cleanup()
{
	int32_t freed = 0;
	for (auto& pool : m_pool)
	{
		T_FATAL_ASSERT(pool.acquired.empty());
		auto it = std::remove_if(pool.free.begin(), pool.free.end(), [](const Texture& texture) {
			return texture.unused > c_maxUnusuedFrames;
		});
		if (it != pool.free.end())
		{
			for (auto it2 = it; it2 != pool.free.end(); ++it2)
			{
				if (it2->texture)
					it2->texture->destroy();
				++freed;
			}
			pool.free.erase(it, pool.free.end());
		}
		for (auto& texture : pool.free)
			texture.unused++;
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
