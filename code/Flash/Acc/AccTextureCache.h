#ifndef traktor_flash_AccTextureCache_H
#define traktor_flash_AccTextureCache_H

#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"
#include "Render/IRenderSystem.h"
#include "Resource/Proxy.h"

namespace traktor
{
	namespace render
	{

class IRenderSystem;
class ISimpleTexture;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace flash
	{

class FlashFillStyle;
class FlashBitmap;

/*! \brief Texture cache for accelerated rendering.
 * \ingroup Flash
 */
class AccTextureCache : public Object
{
public:
	struct BitmapRect
	{
		resource::Proxy< render::ISimpleTexture > texture;
		bool clamp;
		float rect[4];

		BitmapRect()
		{
			clamp = false;
			rect[0] =
			rect[1] =
			rect[2] =
			rect[3] = 0.0f;
		}

		bool operator == (const BitmapRect& rh) const
		{
			if (texture != rh.texture)
				return false;
			if (clamp != rh.clamp)
				return false;

			return
				rect[0] == rh.rect[0] &&
				rect[1] == rh.rect[1] &&
				rect[2] == rh.rect[2] &&
				rect[3] == rh.rect[3];
		}

		bool operator != (const BitmapRect& rh) const
		{
			return !(*this == rh);
		}
	};

	AccTextureCache(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem
	);

	virtual ~AccTextureCache();

	void destroy();

	void clear();

	BitmapRect getGradientTexture(const FlashFillStyle& style);

	BitmapRect getBitmapTexture(const FlashBitmap& bitmap);

private:
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	SmallMap< uint64_t, BitmapRect > m_cache;
};

	}
}

#endif	// traktor_flash_AccTextureCache_H
