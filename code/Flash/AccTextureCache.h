#ifndef traktor_flash_AccTextureCache_H
#define traktor_flash_AccTextureCache_H

#include <map>
#include "Core/Object.h"
#include "Render/RenderSystem.h"

namespace traktor
{
	namespace render
	{

class RenderSystem;
class Texture;

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
	AccTextureCache(render::RenderSystem* renderSystem);

	virtual ~AccTextureCache();

	void destroy();

	void clear();

	render::Texture* getGradientTexture(const FlashFillStyle& style);

	render::Texture* getBitmapTexture(const FlashBitmap& bitmap);

private:
	Ref< render::RenderSystem > m_renderSystem;
	std::map< uint32_t, Ref< render::Texture > > m_cache;
};

	}
}

#endif	// traktor_flash_AccTextureCache_H
