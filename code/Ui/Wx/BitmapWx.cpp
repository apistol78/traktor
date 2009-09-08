#include "Ui/Wx/BitmapWx.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Core/Heap/Ref.h"
#include "Core/Heap/GcNew.h"
#include "Core/Math/Color.h"

namespace traktor
{
	namespace ui
	{

BitmapWx::BitmapWx()
:	m_image(0)
{
}

bool BitmapWx::create(uint32_t width, uint32_t height)
{
	m_image = new wxImage(width, height);
	return true;
}

void BitmapWx::destroy()
{
	if (m_image)
		delete m_image;
}

void BitmapWx::copySubImage(drawing::Image* image, const Rect& srcRect, const Point& destPos)
{
	Ref< drawing::Image > color = image->clone();
	color->convert(drawing::PixelFormat::getB8G8R8());

	memcpy(
		m_image->GetData(),
		color->getData(),
		color->getWidth() * color->getHeight() * 3
	);

	if (image->getPixelFormat()->getAlphaBits())
	{
		Ref< drawing::Image > alpha = image->clone();
		alpha->convert(drawing::PixelFormat::getA8());

		m_image->SetAlpha();
		memcpy(
			m_image->GetAlpha(),
			alpha->getData(),
			alpha->getWidth() * alpha->getHeight()
		);
	}
}

drawing::Image* BitmapWx::getImage() const
{
	const uint8_t* srcC = reinterpret_cast< const uint8_t* >(m_image->GetData());
	const uint8_t* srcA = reinterpret_cast< const uint8_t* >(m_image->GetAlpha());

	if (!srcC)
		return 0;

	Ref< drawing::Image > image;

	if (!srcA)
	{
		image = gc_new< drawing::Image >(
			drawing::PixelFormat::getB8G8R8(),
			m_image->GetWidth(),
			m_image->GetHeight()
		);

		uint8_t* dst = reinterpret_cast< uint8_t* >(image->getData());

		std::memcpy(
			dst,
			srcC,
			m_image->GetWidth() * m_image->GetHeight() * 3
		);
	}
	else
	{
		image = gc_new< drawing::Image >(
			drawing::PixelFormat::getR8G8B8A8(),
			m_image->GetWidth(),
			m_image->GetHeight()
		);

		uint32_t* dst = reinterpret_cast< uint32_t* >(image->getData());

		for (int32_t y = 0; y < image->getHeight(); ++y)
		{
			for (int32_t x = 0; x < image->getWidth(); ++x)
			{
				int32_t offset = x + y * image->getWidth();

				uint32_t r = srcC[offset * 3 + 0];
				uint32_t g = srcC[offset * 3 + 1];
				uint32_t b = srcC[offset * 3 + 2];
				uint32_t a = srcA[offset];

				dst[offset] = (r << 24) | (g << 16) | (b << 8) | a;
			}
		}
	}

	return image;
}

Size BitmapWx::getSize() const
{
	return Size(m_image->GetWidth(), m_image->GetHeight());
}

void BitmapWx::setPixel(uint32_t x, uint32_t y, const Color& color)
{
	m_image->SetRGB(int(x), int(y), color.r, color.g, color.b);
}

Color BitmapWx::getPixel(uint32_t x, uint32_t y) const
{
	return Color(
		m_image->GetRed(int(x), int(y)),
		m_image->GetGreen(int(x), int(y)),
		m_image->GetBlue(int(x), int(y))
	);
}

	}
}
