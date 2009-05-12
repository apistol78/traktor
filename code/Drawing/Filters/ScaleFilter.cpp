#include <cmath>
#include <vector>
#include "Drawing/Filters/ScaleFilter.h"
#include "Drawing/Image.h"
#include "Core/Heap/HeapNew.h"

namespace traktor
{
	namespace drawing
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.ScaleFilter", ScaleFilter, ImageFilter)

ScaleFilter::ScaleFilter(
	uint32_t width,
	uint32_t height,
	MinifyType minify,
	MagnifyType magnify,
	bool keepZeroAlpha
)
:	m_width(width)
,	m_height(height)
,	m_minify(minify)
,	m_magnify(magnify)
,	m_keepZeroAlpha(keepZeroAlpha)
{
}

Image* ScaleFilter::apply(const Image* image)
{
	Ref< Image > final = gc_new< Image >(image->getPixelFormat(), m_width, m_height, image->getPalette());
	Color in, out;

	float sx = image->getWidth() / float(m_width);
	float sy = image->getHeight() / float(m_height);

	std::vector< Color > row(image->getWidth() + 1);

	for (uint32_t y = 0; y < m_height; ++y)
	{
		if (sy < 1.0f)		// Magnify
		{
			if (m_magnify == MgNearest)
			{
				int yy = int(std::floor(y * sy));
				for (uint32_t x = 0; x < image->getWidth(); ++x)
					image->getPixel(x, yy, row[x]);
			}
			else	// MgLinear
			{
				int yy = int(std::floor(y * sy));
				for (uint32_t x = 0; x < image->getWidth(); ++x)
				{
					Color c1, c2;
					image->getPixel(x, yy, c1);
					image->getPixel(x, yy + 1, c2);
					row[x] = c1 + (c2 - c1) * (y * sy - yy);
				}
			}
		}
		else if (sy > 1.0f)	// Minify
		{
			if (m_minify == MnCenter)
			{
				int yy = int(std::floor(y + sy * 0.5f));
				for (uint32_t x = 0; x < image->getWidth(); ++x)
					image->getPixel(x, yy, row[x]);
			}
			else	// MnAverage
			{
				int y1 = int(std::floor(y * sy));
				int y2 = int(std::floor(y * sy + sy));
				for (uint32_t x = 0; x < image->getWidth(); ++x)
				{
					row[x] = Color(0, 0, 0, 0);
					for (int yy = y1; yy < y2; ++yy)
					{
						Color c;
						image->getPixel(x, yy, c);
						row[x] += c;
					}
					row[x] /= float(y2 - y1);
				}
			}
		}
		else	// Keep
		{
			for (uint32_t x = 0; x < image->getWidth(); ++x)
				image->getPixel(x, y, row[x]);
		}

		for (uint32_t x = 0; x < m_width; ++x)
		{
			if (sx < 1.0f)		// Magnify
			{
				if (m_magnify == MgNearest)
				{
					int xx = int(std::floor(x * sx));
					final->setPixel(x, y, row[xx]);
				}
				else	// MgLinear
				{
					int xx = int(std::floor(x * sx));
					final->setPixel(x, y, row[xx] + (row[xx + 1] - row[xx]) * (x * sx - xx));
				}
			}
			else if (sx > 1.0f)	// Minify
			{
				if (m_minify == MnCenter)
				{
					int xx = int(std::floor(x * sx + sx * 0.5f));
					final->setPixel(x, y, row[xx]);
				}
				else	// MnAverage
				{
					int x1 = int(std::floor(x * sx));
					int x2 = std::min< int >(int(std::floor(x * sx + sx)), image->getWidth());

					bool zeroAlpha = false;

					Color c(0, 0, 0, 0);
					for (int xx = x1; xx < x2; ++xx)
					{
						c += row[xx];
						if (row[xx].getAlpha() == 0.0f)
							zeroAlpha = true;
					}

					c /= float(x2 - x1);

					// Keep zero alpha as it's possibly used for masking.
					if (m_keepZeroAlpha && zeroAlpha)
						c.setAlpha(0.0f);

					final->setPixel(x, y, c);
				}
			}
			else	// Keep
			{
				final->setPixel(x, y, row[x]);
			}
		}
	}
	
	return final;
}
	
	}
}
