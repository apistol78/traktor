#include "Flash/FlashBitmapImage.h"
#include "Flash/FlashBitmapResource.h"
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
			false,
			bitmapResource->getX() / w,
			bitmapResource->getY() / h,
			bitmapResource->getWidth() / w,
			bitmapResource->getHeight() / h
		);

		bitmap.setCacheObject(br);
		return br;
	}
	else if (const FlashBitmapImage* bitmapData = dynamic_type_cast< const FlashBitmapImage* >(&bitmap))
	{
		render::SimpleTextureCreateDesc desc;

		desc.width = bitmapData->getWidth();
		desc.height = bitmapData->getHeight();
		desc.mipCount = 1;
		desc.format = render::TfR8G8B8A8;
		desc.immutable = true;
		desc.initialData[0].data = bitmapData->getBits();
		desc.initialData[0].pitch = desc.width * 4;
		desc.initialData[0].slicePitch = desc.width * desc.height * 4;

		resource::Proxy< render::ISimpleTexture > texture = resource::Proxy< render::ISimpleTexture >(m_renderSystem->createSimpleTexture(desc));

		Ref< AccBitmapRect > br = new AccBitmapRect(
			texture,
			false,
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

	}
}
