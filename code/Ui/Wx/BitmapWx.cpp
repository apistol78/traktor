#include "Ui/Wx/BitmapWx.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Core/Heap/Ref.h"
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
	Ref< drawing::Image > image = gc_new< drawing::Image >(
		drawing::PixelFormat::getB8G8R8(),
		m_image->GetWidth(),
		m_image->GetHeight()
	);

	const uint8_t* src = reinterpret_cast< const uint8_t* >(m_image->GetData());
	uint8_t* dst = reinterpret_cast< uint8_t* >(image->getData());

	memcpy(
		dst,
		src,
		m_image->GetWidth() * m_image->GetHeight() * 3
	);

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
