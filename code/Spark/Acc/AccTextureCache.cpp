/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Core/Log/Log.h"
#include "Render/IRenderSystem.h"
#include "Resource/IResourceManager.h"
#include "Resource/Proxy.h"
#include "Spark/BitmapImage.h"
#include "Spark/BitmapResource.h"
#include "Spark/BitmapTexture.h"
#include "Spark/Acc/AccBitmapRect.h"
#include "Spark/Acc/AccTextureCache.h"

namespace traktor::spark
{
	namespace
	{

class AccCachedTexture : public render::ITexture
{
public:
	explicit AccCachedTexture(AccTextureCache* cache, render::ITexture* texture)
	:	m_cache(cache)
	,	m_texture(texture)
	{
	}

	virtual ~AccCachedTexture()
	{
		destroy();
	}

	virtual void destroy() override final
	{
		if (m_texture)
		{
			m_cache->freeTexture(m_texture);
			m_texture = nullptr;
		}
	}

	virtual Size getSize() const override final
	{
		return m_texture->getSize();
	}

	virtual bool lock(int32_t side, int32_t level, Lock& lock) override final
	{
		return m_texture->lock(side, level, lock);
	}

	virtual void unlock(int32_t side, int32_t level) override final
	{
		m_texture->unlock(side, level);
	}

	virtual render::ITexture* resolve() override final
	{
		return m_texture;
	}

private:
	Ref< AccTextureCache > m_cache;
	Ref< render::ITexture > m_texture;
};

	}

AccTextureCache::AccTextureCache(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	bool reuseTextures
)
:	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
,	m_reuseTextures(reuseTextures)
{
}

AccTextureCache::~AccTextureCache()
{
	T_EXCEPTION_GUARD_BEGIN

	destroy();

	T_EXCEPTION_GUARD_END
}

void AccTextureCache::destroy()
{
	m_resourceManager = nullptr;
	m_renderSystem = nullptr;
}

void AccTextureCache::clear()
{
	m_freeTextures.clear();
}

Ref< AccBitmapRect > AccTextureCache::getBitmapTexture(const Bitmap& bitmap)
{
	AccBitmapRect* bmr = static_cast< AccBitmapRect* >(bitmap.getCacheObject());
	if (bmr)
		return bmr;

	if (auto bitmapResource = dynamic_type_cast< const BitmapResource* >(&bitmap))
	{
		resource::Proxy< render::ITexture > texture;

		m_resourceManager->bind(
			resource::Id< render::ITexture >(bitmapResource->getResourceId()),
			texture
		);

		const float w = float(bitmapResource->getAtlasWidth());
		const float h = float(bitmapResource->getAtlasHeight());

		Ref< AccBitmapRect > br = new AccBitmapRect(
			texture,
			bitmapResource->getX() / w,
			bitmapResource->getY() / h,
			bitmapResource->getWidth() / w,
			bitmapResource->getHeight() / h
		);

		bitmap.setCacheObject(br);
		return br;
	}
	else if (auto bitmapTexture = dynamic_type_cast< const BitmapTexture* >(&bitmap))
	{
		Ref< AccBitmapRect > br = new AccBitmapRect(
			resource::Proxy< render::ITexture >(bitmapTexture->getTexture()),
			0.0f,
			0.0f,
			1.0f,
			1.0f
		);

		bitmap.setCacheObject(br);
		return br;
	}
	else if (auto bitmapData = dynamic_type_cast< const BitmapImage* >(&bitmap))
	{
		Ref< render::ITexture > texture;

		// Check if any free texture matching requested size.
		if (m_reuseTextures)
		{
			for (auto i = m_freeTextures.begin(); i != m_freeTextures.end(); ++i)
			{
				const auto sz = i->getSize();
				if (sz.x == bitmapData->getWidth() && sz.y == bitmapData->getHeight())
				{
					texture = *i;
					m_freeTextures.erase(i);
					break;
				}
			}
		}

		// No such texture found; create new texture.
		if (!texture)
		{
			render::SimpleTextureCreateDesc desc;

			desc.width = bitmapData->getWidth();
			desc.height = bitmapData->getHeight();
			desc.mipCount = 1;
			desc.format = render::TfR8G8B8A8;
			desc.immutable = false;

			texture = resource::Proxy< render::ITexture >(m_renderSystem->createSimpleTexture(desc, T_FILE_LINE_W));
		}

		if (!texture)
			return nullptr;

		render::ITexture::Lock tl;
		if (texture->lock(0, 0, tl))
		{
			const uint8_t* s = reinterpret_cast< const uint8_t* >(bitmapData->getBits());
			uint8_t* d = static_cast< uint8_t* >(tl.bits);

			for (uint32_t y = 0; y < bitmapData->getHeight(); ++y)
			{
				std::memcpy(d, s, bitmapData->getWidth() * 4);
				s += bitmapData->getWidth() * 4;
				d += tl.pitch;
			}

			texture->unlock(0, 0);
		}

		Ref< AccBitmapRect > br = new AccBitmapRect(
			resource::Proxy< render::ITexture >(new AccCachedTexture(this, texture)),
			0.0f,
			0.0f,
			1.0f,
			1.0f
		);

		bitmap.setCacheObject(br);
		return br;
	}

	return nullptr;
}

void AccTextureCache::freeTexture(render::ITexture* texture)
{
	if (!texture)
		return;

	if (m_reuseTextures)
		m_freeTextures.push_back(texture);
	else
		texture->destroy();
}

}
