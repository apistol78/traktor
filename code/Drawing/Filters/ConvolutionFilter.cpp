#include "Drawing/Filters/ConvolutionFilter.h"
#include "Drawing/Image.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.ConvolutionFilter", ConvolutionFilter, IImageFilter)

ConvolutionFilter::ConvolutionFilter(const Matrix33& matrix)
:	m_matrix(matrix)
{
}

Ref< ConvolutionFilter > ConvolutionFilter::createGaussianBlur()
{
	return new ConvolutionFilter(Matrix33(
		1, 2, 1,
		2, 4, 2,
		1, 2, 1
	));
}

Ref< ConvolutionFilter > ConvolutionFilter::createEmboss()
{
	return new ConvolutionFilter(Matrix33(
		2,  0,  0,
		0, -1,  0,
		0,  0, -1
	));
}

void ConvolutionFilter::apply(Image* image) const
{
	Ref< Image > final = image->clone(false);
	Color4f in;

	for (int32_t y = 1; y < image->getHeight() - 1; ++y)
	{
		for (int32_t x = 1; x < image->getWidth() - 1; ++x)
		{
			Color4f acc;
			float norm = 0;

			for (int32_t r = 0; r < 3; ++r)
			{
				for (int32_t c = 0; c < 3; ++c)
				{
					image->getPixelUnsafe(x + c - 1, y + r - 1, in);
					acc += in * Scalar(m_matrix.e[r][c]);
					norm += m_matrix.e[r][c];
				}
			}

			if (norm)
				acc /= Scalar(norm);

			final->setPixelUnsafe(x, y, acc);
		}
	}

	image->swap(final);
}
	
	}
}
