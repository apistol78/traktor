#include <cmath>
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Const.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/ScaleFilter.h"

namespace traktor
{
	namespace drawing
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.ScaleFilter", ScaleFilter, IImageFilter)

ScaleFilter::ScaleFilter(
	int32_t width,
	int32_t height,
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

Ref< Image > ScaleFilter::apply(const Image* image)
{
	Ref< Image > final = new Image(image->getPixelFormat(), m_width, m_height, image->getPalette());
	Color4f in, out;
	Color4f c1, c2;

	float sx = image->getWidth() / float(m_width);
	float sy = image->getHeight() / float(m_height);

	AlignedVector< Color4f > row(image->getWidth() + 1, Color4f(0, 0, 0, 0));

	for (int32_t y = 0; y < m_height; ++y)
	{
		if (sy < 1.0f)		// Magnify
		{
			if (m_magnify == MgNearest)
			{
				int yy = int(std::floor(y * sy));
				for (int32_t x = 0; x < image->getWidth(); ++x)
					image->getPixelUnsafe(x, yy, row[x]);
			}
			else	// MgLinear
			{
				int yy = int(std::floor(y * sy));
				Scalar k(y * sy - yy);
				for (int32_t x = 0; x < image->getWidth(); ++x)
				{
					image->getPixelUnsafe(x, yy, c1);
					image->getPixelUnsafe(x, yy + 1, c2);
					row[x] = c1 + (c2 - c1) * k;
				}
			}
		}
		else if (sy > 1.0f)	// Minify
		{
			if (m_minify == MnCenter)
			{
				int yy = int(std::floor(y + sy * 0.5f));
				for (int32_t x = 0; x < image->getWidth(); ++x)
					image->getPixelUnsafe(x, yy, row[x]);
			}
			else	// MnAverage
			{
				int y1 = int(std::floor(y * sy));
				int y2 = int(std::floor(y * sy + sy));
				
				Scalar denom = Scalar(1.0f / float(y2 - y1));
				for (int32_t x = 0; x < image->getWidth(); ++x)
				{
					bool zeroAlpha = false;

					row[x] = Color4f(0, 0, 0, 0);
					if (!m_keepZeroAlpha)
					{
						for (int32_t yy = y1; yy < y2; ++yy)
						{
							image->getPixelUnsafe(x, yy, c1);
							row[x] += c1;
						}
					}
					else
					{
						for (int32_t yy = y1; yy < y2; ++yy)
						{
							image->getPixelUnsafe(x, yy, c1);
							if (c1.getAlpha() <= FUZZY_EPSILON)
								zeroAlpha = true;
							row[x] += c1;
						}
					}
					
					row[x] *= denom;
					
					if (zeroAlpha)
						row[x].setAlpha(Scalar(0.0f));
				}
			}
		}
		else	// Keep
		{
			for (int32_t x = 0; x < image->getWidth(); ++x)
				image->getPixelUnsafe(x, y, row[x]);
		}

		for (int32_t x = 0; x < m_width; ++x)
		{
			if (sx < 1.0f)		// Magnify
			{
				if (m_magnify == MgNearest)
				{
					int32_t xx = int32_t(std::floor(x * sx));
					final->setPixelUnsafe(x, y, row[xx]);
				}
				else	// MgLinear
				{
					int32_t xx = int32_t(std::floor(x * sx));
					final->setPixelUnsafe(x, y, row[xx] + (row[xx + 1] - row[xx]) * Scalar(x * sx - xx));
				}
			}
			else if (sx > 1.0f)	// Minify
			{
				if (m_minify == MnCenter)
				{
					int32_t xx = int32_t(std::floor(x * sx + sx * 0.5f));
					final->setPixelUnsafe(x, y, row[xx]);
				}
				else	// MnAverage
				{
					int32_t x1 = int32_t(std::floor(x * sx));
					int32_t x2 = std::min< int32_t >(int32_t(std::floor(x * sx + sx)), image->getWidth());

					bool zeroAlpha = false;

					Color4f c(0, 0, 0, 0);
					for (int32_t xx = x1; xx < x2; ++xx)
					{
						c += row[xx];
						if (row[xx].getAlpha() <= FUZZY_EPSILON)
							zeroAlpha = true;
					}

					c /= Scalar(float(x2 - x1));

					// Keep zero alpha as it's possibly used for masking.
					if (m_keepZeroAlpha && zeroAlpha)
						c.setAlpha(Scalar(0.0f));

					final->setPixelUnsafe(x, y, c);
				}
			}
			else	// Keep
			{
				final->setPixelUnsafe(x, y, row[x]);
			}
		}
	}
	
	return final;
}
	
	}
}
