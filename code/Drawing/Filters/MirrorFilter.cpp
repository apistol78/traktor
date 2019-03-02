#include <cstring>
#include "Drawing/Filters/MirrorFilter.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.MirrorFilter", MirrorFilter, IImageFilter)

MirrorFilter::MirrorFilter(bool flipHorizontal, bool flipVertical)
:	m_flipHorizontal(flipHorizontal)
,	m_flipVertical(flipVertical)
{
}

void MirrorFilter::apply(Image* image) const
{
	int32_t imageWidth = image->getWidth();
	int32_t imageHeight = image->getHeight();

	int32_t pixelPitch = image->getPixelFormat().getByteSize();
	int32_t rowPitch = imageWidth * pixelPitch;

	Ref< Image > final = image->clone(false);

	const uint8_t* source = static_cast< const uint8_t* >(image->getData());
	uint8_t* destination = static_cast< uint8_t* >(final->getData());

	if (m_flipHorizontal)
	{
		for (int32_t y = 0; y < imageHeight; ++y)
		{
			int32_t dy = m_flipVertical ? (imageHeight - y - 1) : y;
			for (int32_t x = 0; x < imageWidth; ++x)
			{
				int32_t dx = imageWidth - x - 1;
				const uint8_t* sourcePixel = &source[x * pixelPitch + y * rowPitch];
				uint8_t* destinationPixel = &destination[dx * pixelPitch + dy * rowPitch];
				std::memcpy(destinationPixel, sourcePixel, pixelPitch);
			}
		}
	}
	else
	{
		for (int32_t y = 0; y < imageHeight; ++y)
		{
			int32_t dy = m_flipVertical ? (imageHeight - y - 1) : y;
			std::memcpy(&destination[dy * rowPitch], &source[y * rowPitch], rowPitch);
		}
	}

	image->swap(final);
}

	}
}
