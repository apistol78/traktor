#include "Core/Math/Const.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/NormalizeFilter.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.NormalizeFilter", NormalizeFilter, IImageFilter)

void NormalizeFilter::apply(Image* image) const
{
	Color4f in;
	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			image->getPixelUnsafe(x, y, in);

			float nx = in.getRed() * 2.0f - 1.0f;
			float ny = in.getGreen() * 2.0f - 1.0f;
			float nz = in.getBlue() * 2.0f - 1.0f;

			float ln = std::sqrt(nx * nx + ny * ny + nz * nz);

			if (ln >= FUZZY_EPSILON)
			{
				nx /= ln;
				ny /= ln;
				nz /= ln;
			}
			else
			{
				nx = 0.0f;
				ny = 1.0f;
				nz = 0.0f;
			}

			Color4f out(
				nx * 0.5f + 0.5f,
				ny * 0.5f + 0.5f,
				nz * 0.5f + 0.5f,
				in.getAlpha()
			);
			image->setPixelUnsafe(x, y, out);
		}
	}
}

	}
}
