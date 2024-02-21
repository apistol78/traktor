/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Render/IRenderSystem.h"
#include "Render/ITexture.h"
#include "Render/Frame/RenderGraphTexturePool.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderGraphTexturePool", RenderGraphTexturePool, Object)

RenderGraphTexturePool::RenderGraphTexturePool(IRenderSystem* renderSystem)
:	m_renderSystem(renderSystem)
{
}

void RenderGraphTexturePool::destroy()
{
	m_renderSystem = nullptr;
}

Ref< ITexture > RenderGraphTexturePool::acquire(const RenderGraphTextureDesc& textureDesc, uint32_t persistentHandle)
{
	auto it = std::find_if(
		m_pool.begin(),
		m_pool.end(),
		[&](const RenderGraphTexturePool::Pool& p)
		{
			if (p.persistentHandle != persistentHandle)
				return false;

			return std::memcmp(&p.textureDesc, &textureDesc, sizeof(textureDesc)) == 0;
		}
	);

	// Get or create pool.
	Pool* pool = nullptr;
	if (it != m_pool.end())
		pool = &(*it);
	else
	{
		pool = &m_pool.push_back();
		pool->textureDesc = textureDesc;
		pool->persistentHandle = persistentHandle;
	}

	// Acquire free texture, if no one left we need to create a new texture.
	if (!pool->free.empty())
	{
		Ref< ITexture > texture = pool->free.back();

		pool->free.pop_back();
		pool->acquired.push_back(texture);

		return texture;
	}
	else
	{
		SimpleTextureCreateDesc stcd;
		stcd.width = textureDesc.width;
		stcd.height = textureDesc.height;
		stcd.mipCount = textureDesc.mipCount;
		stcd.format = textureDesc.format;
		stcd.shaderStorage = true;

		Ref< ITexture > texture = m_renderSystem->createSimpleTexture(stcd, T_FILE_LINE_W);
		if (texture)
			pool->acquired.push_back(texture);

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
			pool.free.push_back(texture);
			break;
		}
	}
	texture = nullptr;
}

}
