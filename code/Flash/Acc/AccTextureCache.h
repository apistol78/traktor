#ifndef traktor_flash_AccTextureCache_H
#define traktor_flash_AccTextureCache_H

#include "Core/Object.h"

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

class AccBitmapRect;
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

	Ref< AccBitmapRect > getBitmapTexture(const FlashBitmap& bitmap);

private:
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
};

	}
}

#endif	// traktor_flash_AccTextureCache_H
