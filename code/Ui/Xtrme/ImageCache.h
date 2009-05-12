#ifndef traktor_ui_xtrme_ImageCache_H
#define traktor_ui_xtrme_ImageCache_H

#include <map>
#include "Core/Heap/Ref.h"
#include "Core/Object.h"

namespace traktor
{
	namespace render
	{

class RenderSystem;
class Texture;

	}

	namespace ui
	{

class IBitmap;

		namespace xtrme
		{

class ImageCache : public Object
{
	T_RTTI_CLASS(ImageCache)

public:
	ImageCache(render::RenderSystem* renderSystem);

	virtual ~ImageCache();
	
	render::Texture* getTexture(IBitmap* bitmap);

	void flush();
	
private:
	struct CachedTexture
	{
		uint32_t touched;
		Ref< render::Texture > texture;
	};

	Ref< render::RenderSystem > m_renderSystem;
	std::map< IBitmap*, CachedTexture > m_cache;
};

		}
	}
}

#endif	// traktor_ui_xtrme_ImageCache_H
