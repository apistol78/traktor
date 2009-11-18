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

Ref< Image > ConvolutionFilter::apply(const Image* image)
{
	Ref< Image > final = new Image(image->getPixelFormat(), image->getWidth(), image->getHeight(), image->getPalette());
	Color in;

	for (int32_t y = 1; y < image->getHeight() - 1; ++y)
	{
		for (int32_t x = 1; x < image->getWidth() - 1; ++x)
		{
			Color acc;
			float norm = 0;

			for (int32_t r = 0; r < 3; ++r)
			{
				for (int32_t c = 0; c < 3; ++c)
				{
					image->getPixel(x + c - 1, y + r - 1, in);
					acc += in * m_matrix.e[r][c];
					norm += m_matrix.e[r][c];
				}
			}

			if (norm)
				acc /= norm;

			final->setPixel(x, y, acc);
		}
	}

	return final;
}
	
	}
}
