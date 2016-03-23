#ifndef traktor_flash_AccTextureCache_H
#define traktor_flash_AccTextureCache_H

#include "Core/Object.h"
#include "Core/RefArray.h"

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

	void clear();

	Ref< AccBitmapRect > getBitmapTexture(const FlashBitmap& bitmap);

	void freeTexture(render::ISimpleTexture* texture);

private:
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	RefArray< render::ISimpleTexture > m_freeTextures;
};

	}
}

#endif	// traktor_flash_AccTextureCache_H
