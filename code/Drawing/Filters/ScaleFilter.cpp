#include <cmath>
#include <limits>
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

void ScaleFilter::apply(Image* image) const
{
	Ref< Image > final = new Image(image->getPixelFormat(), m_width, m_height, image->getPalette());
	Color4f in, out;
	Color4f c1, c2;

	const int32_t imageWidth = image->getWidth();
	const int32_t imageHeight = image->getHeight();

	const float sx = imageWidth / float(m_width);
	const float sy = imageHeight / float(m_height);

	AlignedVector< Color4f > span(imageWidth + 1, Color4f(0, 0, 0, 0));
	AlignedVector< Color4f > row(imageWidth + 1, Color4f(0, 0, 0, 0));

	for (int32_t y = 0; y < m_height; ++y)
	{
		if (sy < 1.0f)		// Magnify
		{
			if (m_magnify == MgNearest)
			{
				int32_t yy = int32_t(std::floor(y * sy));
				image->getSpanUnsafe(yy, &row[0]);
			}
			else	// MgLinear
			{
				int32_t yy = int32_t(std::floor(y * sy));
				int32_t yn = std::min(yy + 1, imageHeight - 1);

				image->getSpanUnsafe(yy, &row[0]);
				image->getSpanUnsafe(yn, &span[0]);

				Scalar k(y * sy - yy);
				for (int32_t x = 0; x < imageWidth; ++x)
					row[x] = row[x] + (span[x] - row[x]) * k;
			}
		}
		else if (sy > 1.0f)	// Minify
		{
			if (m_minify == MnCenter)
			{
				int32_t yy = int32_t(std::floor(y + sy * 0.5f));
				image->getSpanUnsafe(yy, &row[0]);
			}
			else	// MnAverage
			{
				int32_t y1 = int32_t(std::floor(y * sy));
				int32_t y2 = int32_t(std::floor(y * sy + sy));

				image->getSpanUnsafe(y1, &row[0]);

				if (m_keepZeroAlpha)
				{
					for (int32_t x = 0; x < imageWidth; ++x)
					{
						if (row[x].getAlpha() <= FUZZY_EPSILON)
							row[x].setAlpha(Scalar(-std::numeric_limits< float >::max()));
					}
				}

				for (int32_t yy = y1 + 1; yy < y2; ++yy)
				{
					image->getSpanUnsafe(yy, &span[0]);
					if (!m_keepZeroAlpha)
					{
						for (int32_t x = 0; x < imageWidth; ++x)
							row[x] += span[x];
					}
					else
					{
						for (int32_t x = 0; x < imageWidth; ++x)
						{
							row[x] += span[x];
							if (span[x].getAlpha() <= FUZZY_EPSILON)
								row[x].setAlpha(Scalar(-std::numeric_limits< float >::max()));
						}
					}
				}

				Scalar denom = Scalar(1.0f / float(y2 - y1));
				for (int32_t x = 0; x < imageWidth; ++x)
					row[x] *= denom;

				if (m_keepZeroAlpha)
				{
					for (int32_t x = 0; x < imageWidth; ++x)
					{
						if (row[x].getAlpha() < 0.0f)
							row[x].setAlpha(Scalar(0.0f));
					}
				}
			}
		}
		else	// Keep
		{
			image->getSpanUnsafe(y, &row[0]);
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
					int32_t xn = std::min(xx + 1, imageWidth - 1);
					final->setPixelUnsafe(x, y, row[xx] + (row[xn] - row[xx]) * Scalar(x * sx - xx));
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
					int32_t x2 = std::min< int32_t >(int32_t(std::floor(x * sx + sx)), imageWidth);

					bool zeroAlpha = false;

					Color4f c(0, 0, 0, 0);
					if (!m_keepZeroAlpha)
					{
						for (int32_t xx = x1; xx < x2; ++xx)
							c += row[xx];
					}
					else
					{
						for (int32_t xx = x1; xx < x2; ++xx)
						{
							c += row[xx];
							if (row[xx].getAlpha() <= FUZZY_EPSILON)
								zeroAlpha = true;
						}
					}

					c /= Scalar(float(x2 - x1));

					if (zeroAlpha)
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

	image->swap(final);
}
	
	}
}
