#include "Core/Containers/AlignedVector.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/PremultiplyAlphaFilter.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.PremultiplyAlphaFilter", PremultiplyAlphaFilter, IImageFilter)

void PremultiplyAlphaFilter::apply(Image* image) const
{
	Color4f in;

	int32_t width = image->getWidth();
	int32_t height = image->getHeight();

	AlignedVector< Color4f > span(width);

	for (int32_t y = 0; y < height; ++y)
	{
		image->getSpanUnsafe(y, &span[0]);

		int32_t x = 0;
		for (; x < width - 8; x += 8)
		{
			span[x + 0] = span[x + 0] * span[x + 0].getAlpha();
			span[x + 1] = span[x + 1] * span[x + 1].getAlpha();
			span[x + 2] = span[x + 2] * span[x + 2].getAlpha();
			span[x + 3] = span[x + 3] * span[x + 3].getAlpha();
			span[x + 4] = span[x + 4] * span[x + 4].getAlpha();
			span[x + 5] = span[x + 5] * span[x + 5].getAlpha();
			span[x + 6] = span[x + 6] * span[x + 6].getAlpha();
			span[x + 7] = span[x + 7] * span[x + 7].getAlpha();
		}
		for (; x < width; ++x)
			span[x] = span[x] * span[x].getAlpha();

		image->setSpanUnsafe(y, &span[0]);
	}
}
	
	}
}
