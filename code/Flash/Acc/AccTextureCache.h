#ifndef traktor_flash_AccTextureCache_H
#define traktor_flash_AccTextureCache_H

#include <map>
#include "Core/Object.h"
#include "Render/IRenderSystem.h"

namespace traktor
{
	namespace render
	{

class IRenderSystem;
class ITexture;

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
	AccTextureCache(render::IRenderSystem* renderSystem);

	virtual ~AccTextureCache();

	void destroy();

	void clear();

	Ref< render::ITexture > getGradientTexture(const FlashFillStyle& style);

	Ref< render::ITexture > getBitmapTexture(const FlashBitmap& bitmap);

private:
	Ref< render::IRenderSystem > m_renderSystem;
	std::map< uint64_t, Ref< render::ITexture > > m_cache;
};

	}
}

#endif	// traktor_flash_AccTextureCache_H
