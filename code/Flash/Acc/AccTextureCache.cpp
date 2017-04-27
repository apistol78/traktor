/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstring>
#include "Core/Log/Log.h"
#include "Flash/FlashBitmapImage.h"
#include "Flash/FlashBitmapResource.h"
#include "Flash/FlashBitmapTexture.h"
#include "Flash/Acc/AccBitmapRect.h"
#include "Flash/Acc/AccTextureCache.h"
#include "Render/IRenderSystem.h"
#include "Render/ISimpleTexture.h"
#include "Resource/IResourceManager.h"
#include "Resource/Proxy.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

class AccCachedTexture : public render::ISimpleTexture
{
public:
	AccCachedTexture(AccTextureCache* cache, render::ISimpleTexture* texture)
	:	m_cache(cache)
	,	m_texture(texture)
	{
	}

	virtual ~AccCachedTexture()
	{
		destroy();
	}

	virtual void destroy() T_OVERRIDE T_FINAL
	{
		if (m_texture)
		{
			m_cache->freeTexture(m_texture);
			m_texture = 0;
		}
	}

	virtual render::ITexture* resolve() T_OVERRIDE T_FINAL
	{
		return m_texture;
	}

	virtual int getWidth() const T_OVERRIDE T_FINAL
	{
		return m_texture->getWidth();
	}

	virtual int getHeight() const T_OVERRIDE T_FINAL
	{
		return m_texture->getHeight();
	}

	virtual bool lock(int level, Lock& lock) T_OVERRIDE T_FINAL
	{
		return m_texture->lock(level, lock);
	}

	virtual void unlock(int level) T_OVERRIDE T_FINAL
	{
		m_texture->unlock(level);
	}

	virtual void* getInternalHandle() T_OVERRIDE T_FINAL
	{
		return m_texture->getInternalHandle();
	}

private:
	Ref< AccTextureCache > m_cache;
	Ref< render::ISimpleTexture > m_texture;
};

		}

AccTextureCache::AccTextureCache(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem
)
:	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
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
	m_resourceManager = 0;
	m_renderSystem = 0;
}

void AccTextureCache::clear()
{
	m_freeTextures.clear();
}

Ref< AccBitmapRect > AccTextureCache::getBitmapTexture(const FlashBitmap& bitmap)
{
	AccBitmapRect* bmr = static_cast< AccBitmapRect* >(bitmap.getCacheObject());
	if (bmr)
		return bmr;

	if (const FlashBitmapResource* bitmapResource = dynamic_type_cast< const FlashBitmapResource* >(&bitmap))
	{
		resource::Proxy< render::ISimpleTexture > texture;

		m_resourceManager->bind(
			resource::Id< render::ISimpleTexture >(bitmapResource->getResourceId()),
			texture
		);

		float w = float(bitmapResource->getAtlasWidth());
		float h = float(bitmapResource->getAtlasHeight());

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
	else if (const FlashBitmapTexture* bitmapTexture = dynamic_type_cast< const FlashBitmapTexture* >(&bitmap))
	{
		Ref< AccBitmapRect > br = new AccBitmapRect(
			resource::Proxy< render::ISimpleTexture >(bitmapTexture->getTexture()),
			0.0f,
			0.0f,
			1.0f,
			1.0f
		);

		bitmap.setCacheObject(br);
		return br;
	}
	else if (const FlashBitmapImage* bitmapData = dynamic_type_cast< const FlashBitmapImage* >(&bitmap))
	{
		Ref< render::ISimpleTexture > texture;

		// Check if any free texture matching requested size.
		for (RefArray< render::ISimpleTexture >::iterator i = m_freeTextures.begin(); i != m_freeTextures.end(); ++i)
		{
			if (i->getWidth() == bitmapData->getWidth() && i->getHeight() == bitmapData->getHeight())
			{
				texture = *i;
				m_freeTextures.erase(i);
				break;
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

			texture = resource::Proxy< render::ISimpleTexture >(m_renderSystem->createSimpleTexture(desc));
		}

		if (!texture)
			return 0;

		render::ITexture::Lock tl;
		if (texture->lock(0, tl))
		{
			const uint8_t* s = reinterpret_cast< const uint8_t* >(bitmapData->getBits());
			uint8_t* d = static_cast< uint8_t* >(tl.bits);

			for (uint32_t y = 0; y < bitmapData->getHeight(); ++y)
			{
				std::memcpy(d, s, bitmapData->getWidth() * 4);
				s += bitmapData->getWidth() * 4;
				d += tl.pitch;
			}
				
			texture->unlock(0);
		}

		Ref< AccBitmapRect > br = new AccBitmapRect(
			resource::Proxy< render::ISimpleTexture >(new AccCachedTexture(this, texture)),
			0.0f,
			0.0f,
			1.0f,
			1.0f
		);

		bitmap.setCacheObject(br);
		return br;
	}

	return 0;
}

void AccTextureCache::freeTexture(render::ISimpleTexture* texture)
{
	m_freeTextures.push_back(texture);
}

	}
}
