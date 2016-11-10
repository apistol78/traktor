#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Const.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/GaussianBlurFilter.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.GaussianBlurFilter", GaussianBlurFilter, IImageFilter)

GaussianBlurFilter::GaussianBlurFilter(int32_t size)
:	m_size(size | 1)
{
	m_kernel.resize(m_size);

	float sigma = m_size / 4.73f;
	for (int32_t i = 0; i < 10; ++i)
	{
		const float x = m_size / 2.0f;
		float a = 1.0f / sqrtf(TWO_PI * sigma * sigma);
		float weight = a * std::exp(-((x * x) / (2.0f * sigma * sigma)));
		if (weight > 0.01f)
			sigma -= 0.1f;
		else if (weight < 0.001f)
			sigma += 0.01f;
		else
			break;
	}

	const Scalar a(1.0f / sqrtf(TWO_PI * sigma * sigma));
	
	Scalar totalWeight(0.0f);
	for (int32_t i = 0; i < m_size; ++i)
	{
		float x = i - m_size / 2.0f;
		Scalar weight = a * Scalar(std::exp(-((x * x) / (2.0f * sigma * sigma))));
		m_kernel[i] = Scalar(weight);
		totalWeight += weight;
	}
	for (int32_t i = 0; i < m_size; ++i)
		m_kernel[i] /= totalWeight;
}

void GaussianBlurFilter::apply(Image* image) const
{
	Ref< Image > imm = image->clone(false);

	int32_t w = image->getWidth();
	int32_t h = image->getHeight();

	int32_t m = (m_size & ~1) / 2;

	AlignedVector< Color4f > span(std::max(w, h));
	AlignedVector< Color4f > out(std::max(w, h));

	// \tbd Edges not included in blur.

	// Horizontal pass.
	{
		for (int32_t y = 0; y < h; ++y)
		{
			image->getSpanUnsafe(y, span.ptr());

			for (int32_t x = m; x < w - m; ++x)
			{
				out[x] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
				for (int32_t dx = 0; dx < m_size; ++dx)
					out[x] += span[x + dx - m] * m_kernel[dx];
			}

			imm->setSpanUnsafe(y, out.c_ptr());
		}
	}

	// Vertical pass.
	{
		for (int32_t x = 0; x < w; ++x)
		{
			imm->getVerticalSpanUnsafe(x, span.ptr());

			for (int32_t y = m; y < h - m; ++y)
			{
				out[y] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
				for (int32_t dy = 0; dy < m_size; ++dy)
					out[y] += span[y + dy - m] * m_kernel[dy];
			}

			image->setVerticalSpanUnsafe(x, out.c_ptr());
		}
	}
}
	
	}
}
