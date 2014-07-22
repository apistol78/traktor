#include <cstring>
#include "Drawing/Image.h"
#include "Drawing/Filters/ConvolutionFilter.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.ConvolutionFilter", ConvolutionFilter, IImageFilter)

ConvolutionFilter::ConvolutionFilter(const float* matrix, int32_t size)
:	m_size(size)
{
	m_matrix.resize(size * size);
	std::memcpy(&m_matrix[0], matrix, size * size * sizeof(float));
}

Ref< ConvolutionFilter > ConvolutionFilter::createGaussianBlur3()
{
	const float c_kernel[] =
	{
		1, 2, 1,
		2, 4, 2,
		1, 2, 1
	};
	return new ConvolutionFilter(c_kernel, 3);
}

Ref< ConvolutionFilter > ConvolutionFilter::createGaussianBlur5()
{
	const float c_kernel[] =
	{
		2,  4,  5,  4, 2, 
		4,  9, 12,  9, 4,
		5, 12, 15, 12, 5,
		4,  9, 12,  9, 4,
		2,  4,  5,  4, 2
	};
	return new ConvolutionFilter(c_kernel, 5);
}

Ref< ConvolutionFilter > ConvolutionFilter::createEmboss()
{
	const float c_kernel[] =
	{
		2,  0,  0,
		0, -1,  0,
		0,  0, -1
	};
	return new ConvolutionFilter(c_kernel, 3);
}

void ConvolutionFilter::apply(Image* image) const
{
	Ref< Image > final = image->clone(false);
	Color4f in;

	int32_t hs = m_size / 2;

	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			Color4f acc;
			float norm = 0;

			const float* kernel = &m_matrix[0];
			for (int32_t r = -hs; r <= hs; ++r)
			{
				for (int32_t c = -hs; c <= hs; ++c)
				{
					if (image->getPixel(x + c, y + r, in))
						acc += in * Scalar(*kernel);
					norm += *kernel;
					++kernel;
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
