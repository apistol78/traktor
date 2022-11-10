/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Color4ub.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Ui/Win32/BitmapWin32.h"

namespace traktor
{
	namespace ui
	{

std::atomic< int32_t > BitmapWin32::ms_nextTag(1);

BitmapWin32::BitmapWin32()
{
	m_tag = ms_nextTag++;
}

bool BitmapWin32::create(uint32_t width, uint32_t height)
{
#if defined(T_USE_GDI)
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

	m_hBitmap = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, &m_bits, NULL, 0);
	m_hBitmapPreMulAlpha = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, &m_bitsPreMulAlpha, NULL, 0);

	DeleteDC(hDC);

	if (!m_hBitmap)
		return false;
#endif

	m_width = width;
	m_height = height;

	m_bits.reset(new uint32_t [m_width * m_height]);
	std::memset(m_bits.ptr(), 0, m_width * m_height * sizeof(uint32_t));

	m_bitsPreMulAlpha.reset(new uint32_t [m_width * m_height]);
	std::memset(m_bitsPreMulAlpha.ptr(), 0, m_width * m_height * sizeof(uint32_t));

	return true;
}

void BitmapWin32::destroy()
{
#if defined(T_USE_GDI)
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
#endif
	delete this;
}

void BitmapWin32::copySubImage(drawing::Image* image, const Rect& srcRect, const Point& destPos)
{
	if (srcRect.left >= image->getWidth() || srcRect.top >= image->getHeight())
		return;
	if (srcRect.right < 0 || srcRect.bottom < 0)
		return;

	Rect rc = srcRect;

	rc.left = std::max< int32_t >(0, rc.left);
	rc.top = std::max< int32_t >(0, rc.top);
	rc.right = std::min< int32_t >(image->getWidth(), rc.right);
	rc.bottom = std::min< int32_t >(image->getHeight(), rc.bottom);

	if (rc.getWidth() <= 0 || rc.getHeight() <= 0)
		return;

	int32_t width = m_width - destPos.x;
	int32_t height = m_height - destPos.y;

	if (width <= 0 || height <= 0)
		return;

	if (rc.getWidth() > width)
		rc.right = rc.left + width;
	if (rc.getHeight() > height)
		rc.bottom = rc.top + height;

	// Convert image into a known pixel format.
	Ref< drawing::Image > color = image->clone();
	color->convert(drawing::PixelFormat::getA8R8G8B8());

	// Extract bits from image.
	const uint32_t* src = (const uint32_t*)color->getData();
	const bool srcHasAlpha = (bool)(image->getPixelFormat().getAlphaBits() > 0);
	uint32_t* dstColor = m_bits.ptr();
	uint32_t* dstAlpha = m_bitsPreMulAlpha.ptr();

	for (int32_t y = rc.top; y < rc.bottom; ++y)
	{
		for (int32_t x = rc.left; x < rc.right; ++x)
		{
			uint32_t dstOffset = destPos.x + (x - rc.left) + (m_height - (destPos.y + (y - rc.top)) - 1) * m_width;
			uint32_t c = src[x + y * color->getWidth()];

			// Do not copy alpha channel into color buffer.
			dstColor[dstOffset] = c & 0x00ffffff;

			// Pre-multiply color with alpha, needed for blending in GDI.
			if (srcHasAlpha)
			{
				uint8_t* h = reinterpret_cast< uint8_t* >(&c);
				h[0] = (h[0] * h[3]) >> 8;
				h[1] = (h[1] * h[3]) >> 8;
				h[2] = (h[2] * h[3]) >> 8;
			}

			if (dstAlpha)
				dstAlpha[dstOffset] = c;
		}
	}

	// As soon as an image with alpha has been copied we tag this system image as containing alpha.
	m_haveAlpha |= srcHasAlpha;

	// Increment revision of system image.
	m_revision++;
}

Ref< drawing::Image > BitmapWin32::getImage() const
{
	Ref< drawing::Image > image = new drawing::Image(drawing::PixelFormat::getA8R8G8B8(), m_width, m_height);

	const uint32_t* srcColor = m_bits.c_ptr();
	const uint32_t* srcAlpha = m_bitsPreMulAlpha.c_ptr();
	uint32_t* dst = (uint32_t*)image->getData();

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

HICON BitmapWin32::createIcon() const
{
	HDC hScreenDC = GetDC(NULL);

	// Create mask bitmap.
	HDC hMaskDC = CreateCompatibleDC(hScreenDC);
	HBITMAP hMask = CreateCompatibleBitmap(hMaskDC, m_width, m_height);
	HBITMAP hMaskPrev = (HBITMAP)SelectObject(hMaskDC, hMask);

	const uint32_t* src = (const uint32_t*)m_bitsPreMulAlpha.c_ptr();
	for (uint32_t y = 0; y < m_height; ++y)
	{
		for (uint32_t x = 0; x < m_width; ++x)
		{
			uint32_t color = src[x + y * m_width];
			if ((color & 0xff000000) != 0x00000000)
				SetPixel(hMaskDC, x, m_height - y - 1, RGB(0, 0, 0));
			else
				SetPixel(hMaskDC, x, m_height - y - 1, RGB(255, 255, 255));
		}
	}

	SelectObject(hMaskDC, hMaskPrev);

	// Create color bitmap.
	BITMAPINFO bmi;
	std::memset(&bmi, 0, sizeof(bmi));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = m_width;
	bmi.bmiHeader.biHeight = m_height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = m_width * m_height * 4;

	LPVOID bitsPreMulAlpha = nullptr;
	HBITMAP hBitmapPreMulAlpha = CreateDIBSection(hScreenDC, &bmi, DIB_RGB_COLORS, &bitsPreMulAlpha, NULL, 0);
	std::memcpy(bitsPreMulAlpha, m_bitsPreMulAlpha.c_ptr(), m_width * m_height * 4);

	// Create icon.
	ICONINFO ii;
	ii.fIcon = TRUE;
	ii.hbmColor = hBitmapPreMulAlpha;
	ii.hbmMask = hMask;
	ii.xHotspot = 0;
	ii.yHotspot = 0;

	HICON hIcon = CreateIconIndirect(&ii);

	// Cleanup
	DeleteObject(hBitmapPreMulAlpha);
	DeleteObject(hMask);
	DeleteDC(hMaskDC);

	ReleaseDC(NULL, hScreenDC);
	return hIcon;
}

	}
}
