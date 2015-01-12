#include <cstring>
#include "Core/Math/Color4ub.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Ui/Wx/BitmapWx.h"

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
	//Ref< drawing::Image > color = image->clone();
	//color->convert(drawing::PixelFormat::getB8G8R8());

	//std::memcpy(
	//	m_image->GetData(),
	//	color->getData(),
	//	color->getWidth() * color->getHeight() * 3
	//);

	//if (image->getPixelFormat().getAlphaBits())
	//{
	//	Ref< drawing::Image > alpha = image->clone();
	//	alpha->convert(drawing::PixelFormat::getA8());

	//	m_image->SetAlpha();
	//	std::memcpy(
	//		m_image->GetAlpha(),
	//		alpha->getData(),
	//		alpha->getWidth() * alpha->getHeight()
	//	);
	//}

	if (srcRect.left >= int(image->getWidth()) || srcRect.top >= int(image->getHeight()))
		return;
	if (srcRect.right < 0 || srcRect.bottom < 0)
		return;

	Rect rc = srcRect;

	rc.left = std::max< int >(0, rc.left);
	rc.top = std::max< int >(0, rc.top);
	rc.right = std::min< int >(image->getWidth(), rc.right);
	rc.bottom = std::min< int >(image->getHeight(), rc.bottom);

	if (rc.getWidth() <= 0 || rc.getHeight() <= 0)
		return;

	int width = m_image->GetWidth() - destPos.x;
	int height = m_image->GetHeight() - destPos.y;

	if (width <= 0 || height <= 0)
		return;

	if (rc.getWidth() > width)
		rc.right = rc.left + width;
	if (rc.getHeight() > height)
		rc.bottom = rc.top + height;

	Ref< drawing::Image > color = image->clone();
	color->convert(drawing::PixelFormat::getA8B8G8R8());

	const uint8_t* src = reinterpret_cast< const uint8_t* >(color->getData());
	uint8_t* dstColor = reinterpret_cast< uint8_t* >(m_image->GetData());
	uint8_t* dstAlpha = 0;
	
	if (image->getPixelFormat().getAlphaBits())
	{
		m_image->SetAlpha();
		dstAlpha = reinterpret_cast< uint8_t* >(m_image->GetAlpha());
	}

	for (int y = rc.top; y < rc.bottom; ++y)
	{
		for (int x = rc.left; x < rc.right; ++x)
		{
			uint32_t srcOffset = x + y * color->getWidth();
			uint32_t dstOffset = destPos.x + (x - rc.left) + (destPos.y + (y - rc.top)) * m_image->GetWidth();

			const uint8_t* s = &src[srcOffset * 4];

			uint8_t* dc = &dstColor[dstOffset * 3];
			dc[0] = s[0];
			dc[1] = s[1];
			dc[2] = s[2];

			if (dstAlpha)
			{
				uint8_t* da = &dstAlpha[dstOffset];
				da[0] = s[3];
			}
		}
	}
}

Ref< drawing::Image > BitmapWx::getImage() const
{
	const uint8_t* srcC = reinterpret_cast< const uint8_t* >(m_image->GetData());
	const uint8_t* srcA = reinterpret_cast< const uint8_t* >(m_image->GetAlpha());

	if (!srcC)
		return 0;

	Ref< drawing::Image > image;

	if (!srcA)
	{
		image = new drawing::Image(
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
		image = new drawing::Image(
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

void BitmapWx::setPixel(uint32_t x, uint32_t y, const Color4ub& color)
{
	m_image->SetRGB(int(x), int(y), color.r, color.g, color.b);
}

Color4ub BitmapWx::getPixel(uint32_t x, uint32_t y) const
{
	return Color4ub(
		m_image->GetRed(int(x), int(y)),
		m_image->GetGreen(int(x), int(y)),
		m_image->GetBlue(int(x), int(y)),
		m_image->GetAlpha(int(x), int(y))
	);
}

	}
}
