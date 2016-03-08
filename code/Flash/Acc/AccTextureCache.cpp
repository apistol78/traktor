#include <cstring>
#include "Core/Misc/Adler32.h"
#include "Flash/FlashFillStyle.h"
#include "Flash/FlashBitmapImage.h"
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

const uint32_t c_gradientsSize = 32;
const uint32_t c_gradientsWidth = 1024;
const uint32_t c_gradientsHeight = 1024;
const uint32_t c_gradientsColumns = c_gradientsWidth / c_gradientsSize;

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
,	m_currentGradientColumn(0)
,	m_nextGradient(0)
{
	render::SimpleTextureCreateDesc desc;
	desc.width = c_gradientsWidth;
	desc.height = c_gradientsHeight;
	desc.mipCount = 1;
	desc.format = render::TfR8G8B8A8;
	desc.immutable = false;
	m_gradientsTexture = resource::Proxy< render::ISimpleTexture >(m_renderSystem->createSimpleTexture(desc));
	T_FATAL_ASSERT (m_gradientsTexture);

	m_gradientsData.reset(new uint8_t [c_gradientsWidth * c_gradientsHeight * 4]);
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

	m_nextGradient = 0;

	m_cache.clear();
}

AccTextureCache::BitmapRect AccTextureCache::getGradientTexture(const FlashFillStyle& style)
{
	const AlignedVector< FlashFillStyle::ColorRecord >& colorRecords = style.getColorRecords();
	T_ASSERT (colorRecords.size() > 1);

	Adler32 cs;
	cs.begin();
	cs.feed(style.getGradientType());
	for (AlignedVector< FlashFillStyle::ColorRecord >::const_iterator i = colorRecords.begin(); i != colorRecords.end(); ++i)
		cs.feed(*i);
	cs.end();

	uint64_t hash = cs.get();
	SmallMap< uint64_t, BitmapRect >::iterator it = m_cache.find(hash);
	if (it != m_cache.end())
		return it->second;

	if (style.getGradientType() == FlashFillStyle::GtLinear)
	{
		if (m_nextGradient + 1 > c_gradientsHeight)
		{
			m_nextGradient = 0;
			if ((m_currentGradientColumn += c_gradientsSize) > c_gradientsWidth)
				return BitmapRect();
		}

		uint8_t* gd = &m_gradientsData[(m_currentGradientColumn + m_nextGradient * c_gradientsWidth) * 4];
		int32_t x1 = 0;
		for (int32_t i = 1; i < int32_t(colorRecords.size()); ++i)
		{
			int32_t x2 = int32_t(colorRecords[i].ratio * c_gradientsSize);
			for (int32_t x = x1; x < x2; ++x)
			{
				float f = float(x - x1) / (x2 - x1);
				gd[x * 4 + 0] = uint8_t(colorRecords[i - 1].color.red   * (1.0f - f) + colorRecords[i].color.red   * f);
				gd[x * 4 + 1] = uint8_t(colorRecords[i - 1].color.green * (1.0f - f) + colorRecords[i].color.green * f);
				gd[x * 4 + 2] = uint8_t(colorRecords[i - 1].color.blue  * (1.0f - f) + colorRecords[i].color.blue  * f);
				gd[x * 4 + 3] = uint8_t(colorRecords[i - 1].color.alpha * (1.0f - f) + colorRecords[i].color.alpha * f);
			}
			x1 = x2;
		}
		for (; x1 < c_gradientsSize; ++x1)
		{
			gd[x1 * 4 + 0] = colorRecords.back().color.red;
			gd[x1 * 4 + 1] = colorRecords.back().color.green;
			gd[x1 * 4 + 2] = colorRecords.back().color.blue;
			gd[x1 * 4 + 3] = colorRecords.back().color.alpha;
		}

		render::ITexture::Lock lock;
		if (m_gradientsTexture->lock(0, lock))
		{
			uint8_t* dp = static_cast< uint8_t* >(lock.bits);
			for (uint32_t y = 0; y < c_gradientsHeight; ++y)
			{
				std::memcpy(dp, &m_gradientsData[y * c_gradientsWidth * 4], c_gradientsWidth * 4);
				dp += lock.pitch;
			}
			m_gradientsTexture->unlock(0);
		}

		BitmapRect& br = m_cache[hash];
		br.texture = resource::Proxy< render::ISimpleTexture >(m_gradientsTexture);
		br.clamp = true;
		br.rect[0] = m_currentGradientColumn * 1.0f / c_gradientsWidth;
		br.rect[1] = m_nextGradient * 1.0f / c_gradientsHeight + 0.5f / c_gradientsHeight;
		br.rect[2] = 1.0f / c_gradientsColumns - 1.0f / c_gradientsWidth;
		br.rect[3] = 0.0f;

		m_nextGradient += 1;
		return br;
	}
	else if (style.getGradientType() == FlashFillStyle::GtRadial)
	{
		if (m_nextGradient + c_gradientsSize > c_gradientsHeight)
		{
			m_nextGradient = 0;
			if ((m_currentGradientColumn += c_gradientsSize) > c_gradientsWidth)
				return BitmapRect();
		}

		const float s = float(c_gradientsSize) / 2.0f;

		uint8_t* gd = &m_gradientsData[(m_currentGradientColumn + m_nextGradient * c_gradientsWidth) * 4];
		for (int y = 0; y < c_gradientsSize; ++y)
		{
			for (int x = 0; x < c_gradientsSize; ++x)
			{
				float fx = x / s - 1.0f;
				float fy = y / s - 1.0f;
				float f = sqrtf(fx * fx + fy * fy);
				SwfColor c = interpolateGradient(colorRecords, f);
				gd[x * 4 + 0] = c.red;
				gd[x * 4 + 1] = c.green;
				gd[x * 4 + 2] = c.blue;
				gd[x * 4 + 3] = c.alpha;
			}
			gd += c_gradientsWidth * 4;
		}

		render::ITexture::Lock lock;
		if (m_gradientsTexture->lock(0, lock))
		{
			uint8_t* dp = static_cast< uint8_t* >(lock.bits);
			for (uint32_t y = 0; y < c_gradientsHeight; ++y)
			{
				std::memcpy(dp, &m_gradientsData[y * c_gradientsWidth * 4], c_gradientsWidth * 4);
				dp += lock.pitch;
			}
			m_gradientsTexture->unlock(0);
		}

		BitmapRect& br = m_cache[hash];
		br.texture = resource::Proxy< render::ISimpleTexture >(m_gradientsTexture);
		br.clamp = true;
		br.rect[0] = m_currentGradientColumn * 1.0f / c_gradientsWidth + 0.5f / c_gradientsWidth;
		br.rect[1] = m_nextGradient * 1.0f / c_gradientsHeight + 0.5f / c_gradientsHeight;
		br.rect[2] = (c_gradientsSize - 1) / float(c_gradientsWidth);
		br.rect[3] = (c_gradientsSize - 1) / float(c_gradientsHeight);

		m_nextGradient += c_gradientsSize;
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
		br.clamp = false;
		br.rect[0] = bitmapResource->getX() / w;
		br.rect[1] = bitmapResource->getY() / h;
		br.rect[2] = bitmapResource->getWidth() / w;
		br.rect[3] = bitmapResource->getHeight() / h;

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

		BitmapRect& br = m_cache[hash];
		br.texture = texture;
		br.clamp = false;
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
