#include "Flash/FlashFillStyle.h"
#include "Flash/FlashBitmapData.h"
#include "Flash/FlashBitmapResource.h"
#include "Flash/Acc/AccTextureCache.h"
#include "Render/ISimpleTexture.h"
#include "Resource/IResourceManager.h"
#include "Resource/Proxy.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

SwfColor interpolateGradient(const AlignedVector< FlashFillStyle::ColorRecord >& colorRecords, float f)
{
	if (colorRecords.size() <= 1)
		return colorRecords[0].color;

	int i = 0;
	while (i < int(colorRecords.size() - 2) && colorRecords[i + 1].ratio < f)
		++i;

	const FlashFillStyle::ColorRecord& a = colorRecords[i];
	const FlashFillStyle::ColorRecord& b = colorRecords[i + 1];

	if (f <= a.ratio)
		return a.color;

	if (f >= b.ratio)
		return b.color;

	if ((b.ratio - a.ratio) > 0.0f)
		f = (f - a.ratio) / (b.ratio - a.ratio);
	else
		f = 0.0f;

	SwfColor color =
	{
		uint8_t(a.color.red   * (1.0f - f) + b.color.red   * f),
		uint8_t(a.color.green * (1.0f - f) + b.color.green * f),
		uint8_t(a.color.blue  * (1.0f - f) + b.color.blue  * f),
		uint8_t(a.color.alpha * (1.0f - f) + b.color.alpha * f)
	};

	return color;
}

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
	clear();
	m_renderSystem = 0;
}

void AccTextureCache::clear()
{
	for (SmallMap< uint64_t, BitmapRect >::iterator i = m_cache.begin(); i != m_cache.end(); ++i)
		i->second.texture.clear();

	m_cache.clear();
}

AccTextureCache::BitmapRect AccTextureCache::getGradientTexture(const FlashFillStyle& style)
{
	uint64_t hash = reinterpret_cast< uint64_t >(&style);
	SmallMap< uint64_t, BitmapRect >::iterator it = m_cache.find(hash);
	if (it != m_cache.end())
		return it->second;

	const AlignedVector< FlashFillStyle::ColorRecord >& colorRecords = style.getColorRecords();
	T_ASSERT (colorRecords.size() > 1);

	if (style.getGradientType() == FlashFillStyle::GtLinear)
	{
		SwfColor gradientBitmap[256];

		int x1 = 0;
		for (int i = 1; i < int(colorRecords.size()); ++i)
		{
			int x2 = int(colorRecords[i].ratio * 256);
			for (int x = x1; x < x2; ++x)
			{
				float f = float(x - x1) / (x2 - x1);
				SwfColor& c = gradientBitmap[x];
				c.red = uint8_t(colorRecords[i - 1].color.red * (1.0f - f) + colorRecords[i].color.red * f);
				c.green = uint8_t(colorRecords[i - 1].color.green * (1.0f - f) + colorRecords[i].color.green * f);
				c.blue = uint8_t(colorRecords[i - 1].color.blue * (1.0f - f) + colorRecords[i].color.blue * f);
				c.alpha = uint8_t(colorRecords[i - 1].color.alpha * (1.0f - f) + colorRecords[i].color.alpha * f);
			}
			x1 = x2;
		}

		render::SimpleTextureCreateDesc desc;

		desc.width = 256;
		desc.height = 1;
		desc.mipCount = 1;
		desc.format = render::TfR8G8B8A8;
		desc.immutable = true;
		desc.initialData[0].data = gradientBitmap;
		desc.initialData[0].pitch = 256 * 4;

		resource::Proxy< render::ISimpleTexture > texture = resource::Proxy< render::ISimpleTexture >(m_renderSystem->createSimpleTexture(desc));

		BitmapRect& br = m_cache[hash];
		br.texture = texture;
		br.rect[0] = 0.0f;
		br.rect[1] = 0.0f;
		br.rect[2] = 1.0f;
		br.rect[3] = 1.0f;

		return br;
	}
	else if (style.getGradientType() == FlashFillStyle::GtRadial)
	{
		SwfColor gradientBitmap[64 * 64];

		for (int y = 0; y < 64; ++y)
		{
			for (int x = 0; x < 64; ++x)
			{
				float fx = x / 31.5f - 1.0f;
				float fy = y / 31.5f - 1.0f;
				float f = sqrtf(fx * fx + fy * fy);
				gradientBitmap[x + y * 64] = interpolateGradient(colorRecords, f);
			}
		}

		render::SimpleTextureCreateDesc desc;

		desc.width = 64;
		desc.height = 64;
		desc.mipCount = 1;
		desc.format = render::TfR8G8B8A8;
		desc.immutable = true;
		desc.initialData[0].data = gradientBitmap;
		desc.initialData[0].pitch = 64 * 4;

		resource::Proxy< render::ISimpleTexture > texture = resource::Proxy< render::ISimpleTexture >(m_renderSystem->createSimpleTexture(desc));

		BitmapRect& br = m_cache[hash];
		br.texture = texture;
		br.rect[0] = 0.0f;
		br.rect[1] = 0.0f;
		br.rect[2] = 1.0f;
		br.rect[3] = 1.0f;

		return br;
	}

	return BitmapRect();
}

AccTextureCache::BitmapRect AccTextureCache::getBitmapTexture(const FlashBitmap& bitmap)
{
	uint64_t hash = reinterpret_cast< uint64_t >(&bitmap);

	SmallMap< uint64_t, BitmapRect >::iterator it = m_cache.find(hash);
	if (it != m_cache.end())
		return it->second;

	if (const FlashBitmapResource* bitmapResource = dynamic_type_cast< const FlashBitmapResource* >(&bitmap))
	{
		resource::Proxy< render::ISimpleTexture > texture;

		m_resourceManager->bind(
			resource::Id< render::ISimpleTexture >(bitmapResource->getResourceId()),
			texture
		);

		float w = float(bitmapResource->getAtlasWidth());
		float h = float(bitmapResource->getAtlasHeight());

		BitmapRect& br = m_cache[hash];
		br.texture = texture;
		br.rect[0] = bitmapResource->getX() / w;
		br.rect[1] = bitmapResource->getY() / h;
		br.rect[2] = bitmapResource->getWidth() / w;
		br.rect[3] = bitmapResource->getHeight() / h;

		return br;
	}
	else if (const FlashBitmapData* bitmapData = dynamic_type_cast< const FlashBitmapData* >(&bitmap))
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

		BitmapRect& br = m_cache[hash];
		br.texture = texture;
		br.rect[0] = 0.0f;
		br.rect[1] = 0.0f;
		br.rect[2] = 1.0f;
		br.rect[3] = 1.0f;

		return br;
	}

	return BitmapRect();
}

	}
}
