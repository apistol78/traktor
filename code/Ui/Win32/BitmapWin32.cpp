#include "Ui/Win32/BitmapWin32.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Core/Math/Color4ub.h"

#if defined(T_USE_GDI_PLUS)
using namespace Gdiplus;
#endif

namespace traktor
{
	namespace ui
	{

BitmapWin32::BitmapWin32()
:	m_hBitmap(NULL)
,	m_pBits(0)
,	m_width(0)
,	m_height(0)
,	m_mask(0x00000000)
#if defined(T_USE_GDI_PLUS)
,	m_gpBitmap(0)
,	m_gpBits(0)
,	m_gpAlphaAdd(0x00000000)
#endif
{
}

bool BitmapWin32::create(uint32_t width, uint32_t height)
{
	BITMAPINFO bmi;

	HDC hDC = CreateCompatibleDC(NULL);
	if (!hDC)
		return false;

	std::memset(&bmi, 0, sizeof(bmi));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = width * height * 4;

	m_hBitmap = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, &m_pBits, NULL, 0);
	m_hBitmapPreMulAlpha = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, &m_pBitsPreMulAlpha, NULL, 0);

	DeleteDC(hDC);

	if (!m_hBitmap)
		return false;

	m_width = width;
	m_height = height;

	return true;
}

void BitmapWin32::destroy()
{
	if (m_hBitmap)
	{
		DeleteObject(m_hBitmap);
		m_hBitmap = NULL;
	}
	if (m_hBitmapPreMulAlpha)
	{
		DeleteObject(m_hBitmapPreMulAlpha);
		m_hBitmapPreMulAlpha = NULL;
	}
	delete this;
}

void BitmapWin32::copySubImage(drawing::Image* image, const Rect& srcRect, const Point& destPos)
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

	int width = m_width - destPos.x;
	int height = m_height - destPos.y;

	if (width <= 0 || height <= 0)
		return;

	if (rc.getWidth() > width)
		rc.right = rc.left + width;
	if (rc.getHeight() > height)
		rc.bottom = rc.top + height;

	Ref< drawing::Image > color = image->clone();
	color->convert(drawing::PixelFormat::getA8R8G8B8());

	const uint32_t* src = reinterpret_cast< const uint32_t* >(color->getData());
	uint32_t* dstColor = reinterpret_cast< uint32_t* >(m_pBits);
	uint32_t* dstAlpha = reinterpret_cast< uint32_t* >(m_pBitsPreMulAlpha);

	for (int y = rc.top; y < rc.bottom; ++y)
	{
		for (int x = rc.left; x < rc.right; ++x)
		{
			uint32_t dstOffset = destPos.x + (x - rc.left) + (m_height - (destPos.y + (y - rc.top)) - 1) * m_width;

			uint32_t c = src[x + y * color->getWidth()];

			// Do not copy alpha channel into color buffer.
			dstColor[dstOffset] = c & 0x00ffffff;

			// Pre-multiply color with alpha, needed for blending in GDI.
			uint8_t* h = reinterpret_cast< uint8_t* >(&c);
			h[0] = (h[0] * h[3]) >> 8;
			h[1] = (h[1] * h[3]) >> 8;
			h[2] = (h[2] * h[3]) >> 8;

			dstAlpha[dstOffset] = c;
		}
	}

	GdiFlush();

	const uint8_t* data = reinterpret_cast< const uint8_t* >(color->getData());
	if (data)
		m_mask = data[2] | (data[1] << 8) | (data[0] << 16);

#if defined(T_USE_GDI_PLUS)
	m_gpBitmap.release();
	m_gpBits.release();

	m_gpAlphaAdd = (image->getPixelFormat().getAlphaBits() > 0) ? 0x00000000 : 0xff000000;
#endif
}

Ref< drawing::Image > BitmapWin32::getImage() const
{
	Ref< drawing::Image > image = new drawing::Image(drawing::PixelFormat::getA8R8G8B8(), m_width, m_height);

	const uint32_t* srcColor = reinterpret_cast< const uint32_t* >(m_pBits);
	const uint32_t* srcAlpha = reinterpret_cast< const uint32_t* >(m_pBitsPreMulAlpha);
	uint32_t* dst = reinterpret_cast< uint32_t* >(image->getData());

	for (uint32_t y = 0; y < m_height; ++y)
	{
		uint32_t srcOffset = (m_height - y - 1) * m_width;
		uint32_t dstOffset = y * m_width;
		for (uint32_t x = 0; x < m_width; ++x)
			dst[dstOffset + x] = (srcColor[srcOffset + x] & 0x00ffffff) | (srcAlpha[srcOffset + x] & 0xff000000);
	}

	return image;
}

Size BitmapWin32::getSize() const
{
	return Size(int(m_width), int(m_height));
}

void BitmapWin32::setPixel(uint32_t x, uint32_t y, const Color4ub& color)
{
	uint32_t rgba = (color.a << 24) | (color.r << 16) | (color.g << 8) | color.b;

	uint32_t* dstColor = reinterpret_cast< uint32_t* >(m_pBits);
	uint32_t* dstAlpha = reinterpret_cast< uint32_t* >(m_pBitsPreMulAlpha);
	uint32_t offset = x + (m_height - y - 1) * m_width;

	dstColor[offset] = rgba & 0x00ffffff;

	uint8_t* h = reinterpret_cast< uint8_t* >(&rgba);
	h[0] = (h[0] * h[3]) >> 8;
	h[1] = (h[1] * h[3]) >> 8;
	h[2] = (h[2] * h[3]) >> 8;

	dstAlpha[offset] = rgba;

#if defined(T_USE_GDI_PLUS)
	m_gpBitmap.release();
#endif
}

Color4ub BitmapWin32::getPixel(uint32_t x, uint32_t y) const
{
	uint32_t* dstColor = reinterpret_cast< uint32_t* >(m_pBits);
	uint32_t* dstAlpha = reinterpret_cast< uint32_t* >(m_pBitsPreMulAlpha);
	uint32_t offset = x + (m_height - y - 1) * m_width;

	uint32_t rgb = dstColor[offset];
	uint32_t alpha = dstAlpha[offset];

	int r = (rgb & 0x00ff0000) >> 16;
	int g = (rgb & 0x0000ff00) >> 8;
	int b = (rgb & 0x000000ff);
	int a = (alpha & 0xff000000) >> 24;

	return Color4ub(r, g, b, a);
}

HICON BitmapWin32::createIcon() const
{
	HDC hMaskDC = CreateCompatibleDC(NULL);

	HBITMAP hMask = CreateCompatibleBitmap(hMaskDC, m_width, m_height);
	HBITMAP hMaskPrev = (HBITMAP)SelectObject(hMaskDC, hMask);

	const uint32_t* src = reinterpret_cast< const uint32_t* >(m_pBits);
	for (uint32_t y = 0; y < m_height; ++y)
	{
		for (uint32_t x = 0; x < m_width; ++x)
		{
			uint32_t color = src[x + y * m_width];
			if (color == src[0])
				SetPixel(hMaskDC, x, m_height - y - 1, RGB(255, 255, 255));
			else
				SetPixel(hMaskDC, x, m_height - y - 1, RGB(0, 0, 0));
		}
	}

	SelectObject(hMaskDC, hMaskPrev);
	DeleteDC(hMaskDC);

	ICONINFO ii;
	ii.fIcon = TRUE;
	ii.hbmColor = m_hBitmap;
	ii.hbmMask = hMask;
	ii.xHotspot = 0;
	ii.yHotspot = 0;

	HICON hIcon = CreateIconIndirect(&ii);

	DeleteObject(hMask);

	return hIcon;
}

HBITMAP BitmapWin32::createClone() const
{
	BITMAP bm;
	GetObject(m_hBitmap, sizeof(bm), &bm);

	HBITMAP hCloneBitmap = CreateBitmap(m_width, m_height, bm.bmPlanes, bm.bmBitsPixel, NULL);
	if (!hCloneBitmap)
		return NULL;

	HDC hSourceDC = CreateCompatibleDC(NULL);
	HDC hDestDC = CreateCompatibleDC(NULL);

	HGDIOBJ hOldSourceBitmap = SelectObject(hSourceDC, m_hBitmap);
	HGDIOBJ hOldDestBitmap = SelectObject(hDestDC, hCloneBitmap);
	BitBlt(hDestDC, 0, 0, m_width, m_height, hSourceDC, 0, 0, SRCCOPY);
	SelectObject(hSourceDC, hOldSourceBitmap);
	SelectObject(hDestDC, hOldDestBitmap);

	DeleteDC(hSourceDC);
	DeleteDC(hDestDC);

	return hCloneBitmap;
}

#if defined(T_USE_GDI_PLUS)
Gdiplus::Bitmap* BitmapWin32::getGdiPlusBitmap()
{
	if (!m_gpBitmap.ptr())
	{
		m_gpBits.reset(new uint32_t [m_width * m_height]);

		const uint32_t* srcColor = reinterpret_cast< const uint32_t* >(m_pBits);
		const uint32_t* srcAlpha = reinterpret_cast< const uint32_t* >(m_pBitsPreMulAlpha);

		for (uint32_t y = 0; y < m_height; ++y)
		{
			uint32_t srcOffset = (m_height - y - 1) * m_width;
			uint32_t dstOffset = y * m_width;

			for (uint32_t x = 0; x < m_width; ++x)
				m_gpBits[dstOffset + x] = (srcColor[srcOffset + x] & 0x00ffffff) | (srcAlpha[srcOffset + x] & 0xff000000) | m_gpAlphaAdd;
		}

		m_gpBitmap.reset(new Gdiplus::Bitmap(
			m_width,
			m_height,
			m_width * 4,
			PixelFormat32bppARGB,
			(LPBYTE)m_gpBits.ptr()
		));
	}
	return m_gpBitmap.ptr();
}
#endif

	}
}
