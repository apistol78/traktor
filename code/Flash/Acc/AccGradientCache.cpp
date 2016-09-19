#include <cstring>
#include "Core/Misc/Adler32.h"
#include "Flash/FlashFillStyle.h"
#include "Flash/Acc/AccBitmapRect.h"
#include "Flash/Acc/AccGradientCache.h"
#include "Render/IRenderSystem.h"
#include "Render/ISimpleTexture.h"
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

Color4f interpolateGradient(const AlignedVector< FlashFillStyle::ColorRecord >& colorRecords, float f)
{
	if (colorRecords.size() <= 1)
		return colorRecords[0].color;

	int32_t i = 0;
	while (i < int32_t(colorRecords.size() - 2) && colorRecords[i + 1].ratio < f)
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

	return a.color * Scalar(1.0f - f) + b.color * Scalar(f);
}

		}

AccGradientCache::AccGradientCache(render::IRenderSystem* renderSystem)
:	m_renderSystem(renderSystem)
,	m_currentGradientColumn(0)
,	m_nextGradient(0)
,	m_dirty(false)
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

AccGradientCache::~AccGradientCache()
{
	T_EXCEPTION_GUARD_BEGIN

	destroy();

	T_EXCEPTION_GUARD_END
}

void AccGradientCache::destroy()
{
	clear();
	m_renderSystem = 0;
}

void AccGradientCache::clear()
{
	m_nextGradient = 0;
	m_cache.clear();
}

Ref< AccBitmapRect > AccGradientCache::getGradientTexture(const FlashFillStyle& style)
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
	SmallMap< uint64_t, Ref< AccBitmapRect > >::iterator it = m_cache.find(hash);
	if (it != m_cache.end())
		return it->second;

	if (style.getGradientType() == FlashFillStyle::GtLinear)
	{
		if (m_nextGradient + 1 > c_gradientsHeight)
		{
			m_nextGradient = 0;
			if ((m_currentGradientColumn += c_gradientsSize) > c_gradientsWidth)
				return 0;
		}

		uint8_t* gd = &m_gradientsData[(m_currentGradientColumn + m_nextGradient * c_gradientsWidth) * 4];
		int32_t x1 = 0;
		for (int32_t i = 1; i < int32_t(colorRecords.size()); ++i)
		{
			int32_t x2 = int32_t(colorRecords[i].ratio * c_gradientsSize);
			for (int32_t x = x1; x < x2; ++x)
			{
				float f = float(x - x1) / (x2 - x1);
				Color4f c = (colorRecords[i - 1].color * Scalar(1.0f - f) + colorRecords[i].color * Scalar(f)) * Scalar(255.0f);
				gd[x * 4 + 0] = uint8_t(c.getRed());
				gd[x * 4 + 1] = uint8_t(c.getGreen());
				gd[x * 4 + 2] = uint8_t(c.getBlue());
				gd[x * 4 + 3] = uint8_t(c.getAlpha());
			}
			x1 = x2;
		}

		Color4f c = colorRecords.back().color * Scalar(255.0f);
		for (; x1 < c_gradientsSize; ++x1)
		{
			gd[x1 * 4 + 0] = uint8_t(c.getRed());
			gd[x1 * 4 + 1] = uint8_t(c.getGreen());
			gd[x1 * 4 + 2] = uint8_t(c.getBlue());
			gd[x1 * 4 + 3] = uint8_t(c.getAlpha());
		}

		m_cache[hash] = new AccBitmapRect(
			resource::Proxy< render::ISimpleTexture >(m_gradientsTexture),
			m_currentGradientColumn * 1.0f / c_gradientsWidth,
			m_nextGradient * 1.0f / c_gradientsHeight + 0.5f / c_gradientsHeight,
			1.0f / c_gradientsColumns - 1.0f / c_gradientsWidth,
			0.0f
		);

		m_nextGradient += 1;
		m_dirty = true;

		return m_cache[hash];
	}
	else if (style.getGradientType() == FlashFillStyle::GtRadial)
	{
		if (m_nextGradient + c_gradientsSize > c_gradientsHeight)
		{
			m_nextGradient = 0;
			if ((m_currentGradientColumn += c_gradientsSize) > c_gradientsWidth)
				return 0;
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

				Color4f c = interpolateGradient(colorRecords, f) * Scalar(255.0f);
				gd[x * 4 + 0] = uint8_t(c.getRed());
				gd[x * 4 + 1] = uint8_t(c.getGreen());
				gd[x * 4 + 2] = uint8_t(c.getBlue());
				gd[x * 4 + 3] = uint8_t(c.getAlpha());
			}
			gd += c_gradientsWidth * 4;
		}

		m_cache[hash] = new AccBitmapRect(
			resource::Proxy< render::ISimpleTexture >(m_gradientsTexture),
			m_currentGradientColumn * 1.0f / c_gradientsWidth + 0.5f / c_gradientsWidth,
			m_nextGradient * 1.0f / c_gradientsHeight + 0.5f / c_gradientsHeight,
			(c_gradientsSize - 1) / float(c_gradientsWidth),
			(c_gradientsSize - 1) / float(c_gradientsHeight)
		);

		m_nextGradient += c_gradientsSize;
		m_dirty = true;

		return m_cache[hash];
	}

	return 0;
}

void AccGradientCache::synchronize()
{
	if (!m_dirty)
		return;

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
		m_dirty = false;
	}
}

	}
}
