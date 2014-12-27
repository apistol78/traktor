#include "Graphics/Dd7/GraphicsSystemDd7.h"
#include "Graphics/Dd7/SurfaceDd7.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace graphics
	{
		namespace
		{

bool getDDPixelFormat(PixelFormatEnum pixelFormat, DDPIXELFORMAT& ddpf)
{
	memset(&ddpf, 0, sizeof(ddpf));
	ddpf.dwSize = sizeof(ddpf);
	ddpf.dwFlags = DDPF_RGB;
	ddpf.dwRGBBitCount = getColorBits(pixelFormat);
	ddpf.dwRBitMask = getRedMask(pixelFormat);
	ddpf.dwGBitMask = getGreenMask(pixelFormat);
	ddpf.dwBBitMask = getBlueMask(pixelFormat);
	return true;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.graphics.GraphicsSystemDd7", GraphicsSystemDd7, IGraphicsSystem)

bool GraphicsSystemDd7::getDisplayModes(std::vector< DisplayMode >& outDisplayModes) const
{
	return false;
}

bool GraphicsSystemDd7::getCurrentDisplayMode(DisplayMode& outDisplayMode) const
{
	return false;
}

bool GraphicsSystemDd7::create(const CreateDesc& createDesc)
{
	HRESULT hr;

	if (!(m_hWnd = (HWND)createDesc.windowHandle))
		return false;

	hr = DirectDrawCreateEx(NULL, (LPVOID*)&m_dd.getAssign(), IID_IDirectDraw7, NULL);
	if (FAILED(hr))
	{
		log::error << L"Create graphics failed, unable to create DirectDraw 7" << Endl;
		return false;
	}

	if (!getDDPixelFormat(createDesc.pixelFormat, m_ddpf))
	{
		log::error << L"Create graphics failed, unable to determine format of secondary surface" << Endl;
		return false;
	}

	if (createDesc.fullScreen)
	{
		hr = m_dd->SetCooperativeLevel(m_hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT);
		if (FAILED(hr))
		{
			log::error << L"Create graphics failed, unable to set cooperative level" << Endl;
			return false;
		}

		hr = m_dd->SetDisplayMode(
			createDesc.displayMode.width,
			createDesc.displayMode.height,
			createDesc.displayMode.bits,
			0,
			0
		);
		if (FAILED(hr))
		{
			log::error << L"Create graphics failed, unable to set display mode" << Endl;
			return false;
		}

		DDSURFACEDESC2 ddsd;

		std::memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		ddsd.dwBackBufferCount = 1;

		hr = m_dd->CreateSurface(&ddsd, &m_ddsPrimary.getAssign(), NULL);
		if (FAILED(hr))
		{
			log::error << L"Create graphics failed, unable to create primary surface" << Endl;
			return false;
		}

		DDSCAPS2 ddscaps;

		std::memset(&ddscaps, 0, sizeof(ddscaps));
		ddscaps.dwCaps = DDSCAPS_BACKBUFFER;

		m_ddsPrimary->GetAttachedSurface(&ddscaps, &m_ddsSecondary.getAssign());
		if (FAILED(hr))
		{
			log::error << L"Create graphics failed, unable to get attached surface" << Endl;
			return false;
		}

		m_bltFlip = false;
	}
	else	// !createDesc.fullScreen
	{
		hr = m_dd->SetCooperativeLevel(m_hWnd, DDSCL_NORMAL);
		if (FAILED(hr))
		{
			log::error << L"Create graphics failed, unable to set cooperative level" << Endl;
			return false;
		}

		DDSURFACEDESC2 ddsd;

		std::memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

		hr = m_dd->CreateSurface(&ddsd, &m_ddsPrimary.getAssign(), NULL);
		if (FAILED(hr))
		{
			log::error << L"Create graphics failed, unable to create primary surface" << Endl;
			return false;
		}

		ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
		ddsd.dwWidth = createDesc.displayMode.width;
		ddsd.dwHeight = createDesc.displayMode.height;
		ddsd.ddpfPixelFormat = m_ddpf;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;

		hr = m_dd->CreateSurface(&ddsd, &m_ddsSecondary.getAssign(), NULL);
		if (FAILED(hr))
		{
			log::error << L"Create graphics failed, unable to create secondary surface" << Endl;
			return false;
		}

		hr = m_dd->CreateClipper(0, &m_ddClipper.getAssign(), NULL);
		if (FAILED(hr))
		{
			log::error << L"Create graphics failed, unable to create clipper" << Endl;
			return false;
		}

		hr = m_ddClipper->SetHWnd(0, m_hWnd);
		if (FAILED(hr))
		{
			log::error << L"Create graphics failed, unable to set window handle to clipper" << Endl;
			return false;
		}

		hr = m_ddsPrimary->SetClipper(m_ddClipper);
		if (FAILED(hr))
		{
			log::error << L"Create graphics failed, unable to set clipper on primary surface" << Endl;
			return false;
		}

		m_bltFlip = true;
	}

	m_primary = new SurfaceDd7(m_ddsPrimary);
	m_secondary = new SurfaceDd7(m_ddsSecondary);

	return true;
}

void GraphicsSystemDd7::destroy()
{
	m_ddsSecondary.release();
	m_ddsPrimary.release();
	m_ddClipper.release();
	m_dd.release();
}

bool GraphicsSystemDd7::resize(int width, int height)
{
	DDSURFACEDESC2 ddsd;
	HRESULT hr;

	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
	ddsd.dwWidth = width;
	ddsd.dwHeight = height;
	ddsd.ddpfPixelFormat = m_ddpf;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;

	ComRef< IDirectDrawSurface7 > ddsSecondary;
	hr = m_dd->CreateSurface(&ddsd, &ddsSecondary.getAssign(), NULL);
	if (FAILED(hr))
		return false;

	m_secondary->swap(ddsSecondary);
	m_ddsSecondary = ddsSecondary;

	return true;
}

Ref< ISurface > GraphicsSystemDd7::getPrimarySurface()
{
	return m_primary;
}

Ref< ISurface > GraphicsSystemDd7::getSecondarySurface()
{
	return m_secondary;
}

Ref< ISurface > GraphicsSystemDd7::createOffScreenSurface(const SurfaceDesc& surfaceDesc)
{
	DDSURFACEDESC2 ddsd;
	HRESULT hr;

	std::memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
	ddsd.dwWidth = surfaceDesc.width;
	ddsd.dwHeight = surfaceDesc.height;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	if (!getDDPixelFormat(surfaceDesc.pixelFormat, ddsd.ddpfPixelFormat))
		return 0;

	ComRef< IDirectDrawSurface7 > ddsOffScreen;
	hr = m_dd->CreateSurface(&ddsd, &ddsOffScreen.getAssign(), NULL);
	if (FAILED(hr))
		return 0;

	return new SurfaceDd7(ddsOffScreen);
}

void GraphicsSystemDd7::flip(bool waitVBlank)
{
	if (!m_bltFlip)
		m_ddsPrimary->Flip(NULL, waitVBlank ? DDFLIP_WAIT : DDFLIP_NOVSYNC);
	else
	{
		POINT pntTopLeft = { 0, 0 };
		RECT rcTarget;

		ClientToScreen(m_hWnd, &pntTopLeft);
		GetClientRect(m_hWnd, &rcTarget);
		OffsetRect(&rcTarget, pntTopLeft.x, pntTopLeft.y);

		for (;;)
		{
			HRESULT hr = m_ddsPrimary->Blt(&rcTarget, m_ddsSecondary, NULL, DDBLT_WAIT, NULL);
			if (hr == DD_OK)
				break;
			else if (hr == DDERR_SURFACELOST)
			{
				m_ddsPrimary->Restore();
				m_ddsSecondary->Restore();
			}
			else if (hr != DDERR_WASSTILLDRAWING)
				break;
		}
	}
}

	}
}
