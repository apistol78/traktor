/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

	if (image->getPixelFormat().getAlphaBits())
	{
		if (!m_image->HasAlpha())
			m_image->InitAlpha();
	}

	Ref< drawing::Image > color = image->clone();
	color->convert(drawing::PixelFormat::getA8B8G8R8());

	const uint8_t* src = reinterpret_cast< const uint8_t* >(color->getData());
	for (int y = rc.top; y < rc.bottom; ++y)
	{
		for (int x = rc.left; x < rc.right; ++x)
		{
			uint32_t srcOffset = x + y * color->getWidth();
			const uint8_t* s = &src[srcOffset * 4];

			m_image->SetRGB(
				destPos.x + (x - rc.left),
				destPos.y + (y - rc.top),
				s[0],
				s[1],
				s[2]
			);

			if (m_image->HasAlpha())
				m_image->SetAlpha(
					destPos.x + (x - rc.left),
					destPos.y + (y - rc.top),
					s[3]
				);
		}
	}
}

Ref< drawing::Image > BitmapWx::getImage() const
{
	Ref< drawing::Image > image = new drawing::Image(
		drawing::PixelFormat::getR8G8B8A8(),
		m_image->GetWidth(),
		m_image->GetHeight()
	);

	uint32_t* dst = reinterpret_cast< uint32_t* >(image->getData());
	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			uint32_t r = m_image->GetRed(x, y);
			uint32_t g = m_image->GetGreen(x, y);
			uint32_t b = m_image->GetBlue(x, y);
			uint32_t a = m_image->HasAlpha() ? m_image->GetAlpha(x, y) : 255;
			*dst++ = (r << 24) | (g << 16) | (b << 8) | a;
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
