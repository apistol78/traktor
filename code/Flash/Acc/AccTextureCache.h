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
class ITexture;

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
	AccTextureCache(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem
	);

	virtual ~AccTextureCache();

	void destroy();

	void clear();

	Ref< render::ITexture > getGradientTexture(const FlashFillStyle& style);

	Ref< render::ITexture > getBitmapTexture(const FlashBitmap& bitmap);

private:
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	SmallMap< uint64_t, resource::Proxy< render::ITexture > > m_cache;
};

	}
}

#endif	// traktor_flash_AccTextureCache_H
