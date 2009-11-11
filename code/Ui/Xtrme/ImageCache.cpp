#include "Ui/Xtrme/ImageCache.h"
#include "Ui/Itf/IBitmap.h"
#include "Render/IRenderSystem.h"
#include "Render/ISimpleTexture.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/ScaleFilter.h"

namespace traktor
{
	namespace ui
	{
		namespace xtrme
		{
			namespace
			{

const uint32_t c_maxUntouchedCount = 8;	//!< Number of paints texture can be unused before it's automatically flushed from cache.
			
uint32_t nearestLog2(uint32_t v)
{
	for (int32_t i = 31; i >= 0; --i)
	{
		if ((1U << i) < v)
			return 1U << (i + 1);
	}
	return 1;
}

bool isLog2(uint32_t v)
{
	return nearestLog2(v) == v;
}

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.xtrme.ImageCache", ImageCache, Object)

ImageCache::ImageCache(render::IRenderSystem* renderSystem)
:	m_renderSystem(renderSystem)
{
}

ImageCache::~ImageCache()
{
	for (std::map< IBitmap*, CachedTexture >::iterator i = m_cache.begin(); i != m_cache.end(); ++i)
	{
		if (i->second.texture)
			i->second.texture->destroy();
	}
}

Ref< render::ITexture > ImageCache::getTexture(IBitmap* bitmap)
{
	std::map< IBitmap*, CachedTexture >::iterator i = m_cache.find(bitmap);
	if (i != m_cache.end())
	{
		i->second.touched = 0;
		return i->second.texture;
	}

	Ref< drawing::Image > image = bitmap->getImage();
	image->convert(drawing::PixelFormat::getA8B8G8R8());

	uint32_t width = image->getWidth();
	uint32_t height = image->getHeight();
	
#if defined(__APPLE)

	// Ensure image dimensions is a power 2.
	if (!isLog2(width) || !isLog2(height))
	{
		width = nearestLog2(width);
		height = nearestLog2(height);
		drawing::ScaleFilter scaleLog2(
			width,
			height,
			drawing::ScaleFilter::MnCenter,
			drawing::ScaleFilter::MgNearest
		);
		image = image->applyFilter(&scaleLog2);
	}

#endif
	
	render::SimpleTextureCreateDesc desc;

	desc.width = width;
	desc.height = height;
	desc.mipCount = 1;
	desc.format = render::TfR8G8B8A8;
	desc.immutable = true;
	desc.initialData[0].data = image->getData();
	desc.initialData[0].pitch = image->getWidth() * 4;

	Ref< render::ISimpleTexture > texture = m_renderSystem->createSimpleTexture(desc);
	T_ASSERT_M (texture, L"Unable to create texture");

	m_cache[bitmap].touched = 0;
	m_cache[bitmap].texture = texture;
	return texture;
}

void ImageCache::flush()
{
	for (std::map< IBitmap*, CachedTexture >::iterator i = m_cache.begin(); i != m_cache.end(); )
	{
		if (i->second.touched++ >= c_maxUntouchedCount)
		{
			T_ASSERT (i->second.texture);
			i->second.texture->destroy();
			m_cache.erase(i++);
		}
		else
			++i;
	}
}

		}
	}
}