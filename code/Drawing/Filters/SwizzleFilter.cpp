#include <cctype>
#include "Drawing/Filters/SwizzleFilter.h"
#include "Drawing/Image.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.SwizzleFilter", SwizzleFilter, IImageFilter)

SwizzleFilter::SwizzleFilter(const std::wstring& swizzle)
{
	T_ASSERT (swizzle.length() >= 4);
	m_swizzle[0] = swizzle[0];
	m_swizzle[1] = swizzle[1];
	m_swizzle[2] = swizzle[2];
	m_swizzle[3] = swizzle[3];
}

void SwizzleFilter::apply(Image* image) const
{
	Color4f in, out;

	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			image->getPixelUnsafe(x, y, in);
			for (int32_t i = 0; i < 4; ++i)
			{
				switch (std::toupper(m_swizzle[i]))
				{
				case L'A': 
					out.set(i, in.getAlpha());
					break;
				case L'R': 
					out.set(i, in.getRed());
					break;
				case L'G': 
					out.set(i, in.getGreen());
					break;
				case L'B': 
					out.set(i, in.getBlue());
					break;
				case L'0':
					out.set(i, Scalar(0.0f));
					break;
				case L'1':
					out.set(i, Scalar(1.0f));
					break;
				}
			}
			image->setPixelUnsafe(x, y, out);
		}
	}
}

	}
}
