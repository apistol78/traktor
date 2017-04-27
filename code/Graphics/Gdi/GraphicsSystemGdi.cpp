/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#define _WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Core/Log/Log.h"
#include "Graphics/Gdi/GraphicsSystemGdi.h"
#include "Graphics/Gdi/SurfaceGdi.h"

namespace traktor
{
	namespace graphics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.graphics.GraphicsSystemGdi", GraphicsSystemGdi, IGraphicsSystem)

bool GraphicsSystemGdi::getDisplayModes(std::vector< DisplayMode >& outDisplayModes) const
{
	DEVMODE devm;

	std::memset(&devm, 0, sizeof(devm));
	for (DWORD i = 0; EnumDisplaySettings(NULL, i, &devm) != 0; ++i)
	{
		DisplayMode dm;
		dm.width = devm.dmPelsWidth;
		dm.height = devm.dmPelsHeight;
		dm.bits = devm.dmBitsPerPel;
		outDisplayModes.push_back(dm);
	}

	return true;
}

bool GraphicsSystemGdi::getCurrentDisplayMode(DisplayMode& outDisplayMode) const
{
	DEVMODE devm;

	std::memset(&devm, 0, sizeof(devm));
	if (!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devm))
		return false;

	outDisplayMode.width = devm.dmPelsWidth;
	outDisplayMode.height = devm.dmPelsHeight;
	outDisplayMode.bits = devm.dmBitsPerPel;

	return true;
}

bool GraphicsSystemGdi::create(const CreateDesc& createDesc)
{
	m_hWnd = createDesc.syswin.hWnd;
	if (!m_hWnd)
		return false;

	// Ensure client size match resolution.

	// Get window dc.
	m_hDC = GetDC(m_hWnd);
	if (!m_hDC)
		return false;

	m_secondaryFormat = createDesc.pixelFormat;

	return resize(createDesc.displayMode.width, createDesc.displayMode.height);
}

void GraphicsSystemGdi::destroy()
{
	m_secondarySurface = 0;
	m_primarySurface = 0;

	if (m_hDC)
	{
		ReleaseDC(m_hWnd, m_hDC);
		m_hWnd = 0;
		m_hDC = 0;
	}
}

bool GraphicsSystemGdi::resize(int width, int height)
{
	SurfaceDesc surfaceDesc;

	// Create bitmap header.
	for (uint32_t i = 0; i < sizeof_array(m_bitmapbuffer); i++)
		m_bitmapbuffer[i] = 0;
	m_bitmapHeader = (BITMAPINFO*)&m_bitmapbuffer;
	m_bitmapHeader->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_bitmapHeader->bmiHeader.biPlanes = 1;
	m_bitmapHeader->bmiHeader.biBitCount = 32;
	m_bitmapHeader->bmiHeader.biCompression = BI_BITFIELDS;
	m_bitmapHeader->bmiHeader.biWidth = width;
	m_bitmapHeader->bmiHeader.biHeight = -height;
	((uint32_t*)m_bitmapHeader->bmiColors)[0] = 0x00FF0000;
	((uint32_t*)m_bitmapHeader->bmiColors)[1] = 0x0000FF00;
	((uint32_t*)m_bitmapHeader->bmiColors)[2] = 0x000000FF;

	// Create primary surface.
	surfaceDesc.width = width;
	surfaceDesc.height = height;
	surfaceDesc.pixelFormat = PfeA8R8G8B8;

	m_primarySurface = checked_type_cast< SurfaceGdi* >(createOffScreenSurface(surfaceDesc));
	if (!m_primarySurface)
		return false;

	// Create secondary surface.
	surfaceDesc.width = width;
	surfaceDesc.height = height;
	surfaceDesc.pixelFormat = m_secondaryFormat;

	m_secondarySurface = checked_type_cast< SurfaceGdi* >(createOffScreenSurface(surfaceDesc));
	if (!m_secondarySurface)
		return false;

	return true;
}

Ref< ISurface > GraphicsSystemGdi::getPrimarySurface()
{
	return m_primarySurface;
}

Ref< ISurface > GraphicsSystemGdi::getSecondarySurface()
{
	return m_secondarySurface;
}

Ref< ISurface > GraphicsSystemGdi::createOffScreenSurface(const SurfaceDesc& surfaceDesc)
{
	return new SurfaceGdi(surfaceDesc);
}

void GraphicsSystemGdi::flip(bool waitVBlank)
{
	flip(m_hDC);
}

void GraphicsSystemGdi::flip(HDC hDC)
{
	SurfaceDesc primarySurfaceDesc;
	void* primaryMemory = m_primarySurface->lock(primarySurfaceDesc);

	SurfaceDesc secondarySurfaceDesc;
	void* secondaryMemory = m_secondarySurface->lock(secondarySurfaceDesc);

	// Convert and copy from secondary to primary.

	// 565
	if (secondarySurfaceDesc.pixelFormat == PfeR5G6B5)
	{
		uint32_t* pm = static_cast< uint32_t* >(primaryMemory);
		uint16_t* sm = static_cast< uint16_t* >(secondaryMemory);

		for (uint32_t y = 0; y < primarySurfaceDesc.height; ++y)
		{
			for (uint32_t x = 0; x < primarySurfaceDesc.width; ++x)
			{
				uint16_t c = sm[x];

				uint32_t r = (c & 0xf800) >> 8;
				uint32_t g = (c & 0x07e0) >> 3;
				uint32_t b = (c & 0x001f) << 3;

				pm[x] = 0xff000000 | (r << 16) | (g << 8) | b;
			}

			pm += primarySurfaceDesc.pitch / 4;
			sm += secondarySurfaceDesc.pitch / 2;
		}
	}
	else if (secondarySurfaceDesc.pixelFormat == PfeA8R8G8B8)
	{
		uint32_t* pm = static_cast< uint32_t* >(primaryMemory);
		uint32_t* sm = static_cast< uint32_t* >(secondaryMemory);

		for (uint32_t y = 0; y < primarySurfaceDesc.height; ++y)
		{
			for (uint32_t x = 0; x < primarySurfaceDesc.width; ++x)
			{
				uint32_t c = sm[x];
				pm[x] = 0xff000000 | c;
			}

			pm += primarySurfaceDesc.pitch / 4;
			sm += secondarySurfaceDesc.pitch / 4;
		}
	}

	// Blit primary bits.
	RECT rc;
	GetClientRect(m_hWnd, &rc);
	StretchDIBits(
		hDC,
		0,
		0,
		rc.right,
		rc.bottom,
		0,
		0,
		m_bitmapHeader->bmiHeader.biWidth,
		-m_bitmapHeader->bmiHeader.biHeight,
		primaryMemory,
		m_bitmapHeader,
		DIB_RGB_COLORS,
		SRCCOPY
	);

	m_secondarySurface->unlock();
	m_primarySurface->unlock();
}

	}
}
