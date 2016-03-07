#include "Core/Math/Const.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/BlurFilter.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.BlurFilter", BlurFilter, IImageFilter)

BlurFilter::BlurFilter(int32_t x, int32_t y)
:	m_x(x)
,	m_y(y)
{
}

void BlurFilter::apply(Image* image) const
{
	Ref< Image > imm = image->clone(false);
	Color4f tmp;

	// Horizontal pass.
	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			Color4f acc;
			for (int32_t dx = -m_x; dx <= m_x; ++dx)
			{
				if (!image->getPixel(x + dx, y, tmp))
					image->getPixel(x, y, tmp);
				acc += tmp;
			}
			acc /= Scalar(m_x * 2 + 1);
			imm->setPixelUnsafe(x, y, acc);
		}
	}

	// Vertical pass.
	for (int32_t x = 0; x < image->getWidth(); ++x)
	{
		for (int32_t y = 0; y < image->getHeight(); ++y)
		{
			Color4f acc;
			for (int32_t dy = -m_y; dy <= m_y; ++dy)
			{
				if (!imm->getPixel(x, y + dy, tmp))
					imm->getPixel(x, y, tmp);
				acc += tmp;
			}
			acc /= Scalar(m_y * 2 + 1);
			image->setPixelUnsafe(x, y, acc);
		}
	}
}

	}
}
