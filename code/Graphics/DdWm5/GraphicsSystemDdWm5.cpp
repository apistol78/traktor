#include "Core/Log/Log.h"
#include "Graphics/DdWm5/GraphicsSystemDdWm5.h"
#include "Graphics/DdWm5/SurfaceDdWm5.h"
#include "Graphics/DdWm5/SurfaceOffscreen.h"

namespace traktor
{
	namespace graphics
	{
		namespace
		{

const wchar_t* translateError(HRESULT hr)
{
	switch (hr)
	{
	case DDERR_GENERIC:
		return L"Generic error";
	case DDERR_INVALIDMODE:
		return L"Invalid mode";
	case DDERR_INVALIDOBJECT:
		return L"Invalid object";
	case DDERR_INVALIDPARAMS:
		return L"Invalid parameters";
	case DDERR_LOCKEDSURFACES:
		return L"Locked surfaces";
	case DDERR_SURFACEBUSY:
		return L"Surface busy";
	case DDERR_UNSUPPORTED:
		return L"Unsupported";
	case DDERR_WASSTILLDRAWING:
		return L"Was still drawing";
	}
	return L"Unknown error";
}

HRESULT WINAPI enumDisplayModeCallback(LPDDSURFACEDESC ddsd, LPVOID context)
{
	std::vector< DisplayMode >& outDisplayModes = *(std::vector< DisplayMode >*)context;
	DisplayMode dm =
	{
		ddsd->dwWidth,
		ddsd->dwHeight,
		uint8_t(ddsd->ddpfPixelFormat.dwRGBBitCount)
	};
	outDisplayModes.push_back(dm);
	return S_OK;
}

struct GetBackSurfaceEnum
{
	ComRef< IDirectDrawSurface > m_ddBackSurface;

	static HRESULT WINAPI callback(LPDIRECTDRAWSURFACE lpDDSurface, LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext)
	{
		GetBackSurfaceEnum* this_ = reinterpret_cast< GetBackSurfaceEnum* >(lpContext);
		this_->m_ddBackSurface = lpDDSurface;
		return DDENUMRET_CANCEL;
	}
};

bool getDDPixelFormat(PixelFormatEnum pixelFormat, DDPIXELFORMAT& ddpf)
{
	std::memset(&ddpf, 0, sizeof(ddpf));
	ddpf.dwSize = sizeof(ddpf);
	ddpf.dwFlags = DDPF_RGB;
	ddpf.dwRGBBitCount = getColorBits(pixelFormat);
	ddpf.dwRBitMask = getRedMask(pixelFormat);
	ddpf.dwGBitMask = getGreenMask(pixelFormat);
	ddpf.dwBBitMask = getBlueMask(pixelFormat);
	return true;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.graphics.GraphicsSystemDdWm5", GraphicsSystemDdWm5, IGraphicsSystem)

GraphicsSystemDdWm5::GraphicsSystemDdWm5()
:	m_hWnd(NULL)
{
	HRESULT hr = DirectDrawCreate(NULL, (IDirectDraw**)&m_dd.getAssign(), NULL);
	T_ASSERT_M (SUCCEEDED(hr), translateError(hr));
}

bool GraphicsSystemDdWm5::getDisplayModes(std::vector< DisplayMode >& outDisplayModes) const
{
	m_dd->EnumDisplayModes(0, NULL, (LPVOID)&outDisplayModes, enumDisplayModeCallback);
	return true;
}

bool GraphicsSystemDdWm5::getCurrentDisplayMode(DisplayMode& outDisplayMode) const
{
	return false;
}

bool GraphicsSystemDdWm5::create(const CreateDesc& createDesc)
{
	DDSURFACEDESC ddsd;
	HRESULT hr;

	if (!(m_hWnd = (HWND)createDesc.windowHandle))
		return false;

	if (!getDDPixelFormat(createDesc.pixelFormat, m_ddPixelFormat))
		return false;

	if (createDesc.fullScreen)
	{
		hr = m_dd->SetCooperativeLevel(m_hWnd, DDSCL_FULLSCREEN);
		if (FAILED(hr))
		{
			log::error << L"Create graphics failed; unable to set cooperative level (" << translateError(hr) << L")" << Endl;
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
			log::warning << L"Create graphics failed; unable to set display mode (" << translateError(hr) << L")" << Endl;
			return false;
		}
	}
	else
	{
		hr = m_dd->SetCooperativeLevel(m_hWnd, DDSCL_NORMAL);
		if (FAILED(hr))
		{
			log::error << L"Create graphics failed; unable to set cooperative level (" << translateError(hr) << L")" << Endl;
			return false;
		}
	}
	std::memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	hr = m_dd->CreateSurface(&ddsd, &m_ddsPrimary.getAssign(), NULL);
	if (FAILED(hr))
	{
		log::error << L"Create graphics failed; unable to create primary surface (" << translateError(hr) << L")" << Endl;
		return false;
	}

	std::memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
	ddsd.dwWidth = createDesc.displayMode.width;
	ddsd.dwHeight = createDesc.displayMode.height;
	ddsd.ddpfPixelFormat = m_ddPixelFormat;

	hr = m_dd->CreateSurface(&ddsd, &m_ddsSecondary.getAssign(), NULL);
	if (FAILED(hr))
	{
		log::error << L"Create graphics failed; unable to create secondary surface (" << translateError(hr) << L")" << Endl;
		return false;
	}

	if (!createDesc.fullScreen)
	{
		hr = m_dd->CreateClipper(0, &m_ddcClipper.getAssign(), NULL);
		if (FAILED(hr))
		{
			log::error << L"Create graphics failed; unable to create clipper" << Endl;
			return false;
		}

		hr = m_ddcClipper->SetHWnd(0, m_hWnd);
		if (FAILED(hr))
		{
			log::error << L"Create graphics failed, unable to set window handle to clipper" << Endl;
			return false;
		}

		hr = m_ddsPrimary->SetClipper(m_ddcClipper);
		if (FAILED(hr))
		{
			log::error << L"Create graphics failed, unable to set clipper on primary surface" << Endl;
			return false;
		}
	}

	m_primary = new SurfaceDdWm5(m_ddsPrimary);
	m_secondary = new SurfaceDdWm5(m_ddsSecondary);
	return true;
}

void GraphicsSystemDdWm5::destroy()
{
	m_ddcClipper.release();
	m_ddsPrimary.release();
	m_ddsSecondary.release();
	m_dd.release();
}

bool GraphicsSystemDdWm5::resize(int width, int height)
{
	DDSURFACEDESC ddsd;
	HRESULT hr;

	std::memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
	ddsd.dwWidth = width;
	ddsd.dwHeight = height;
	ddsd.ddpfPixelFormat = m_ddPixelFormat;

	hr = m_dd->CreateSurface(&ddsd, &m_ddsSecondary.getAssign(), NULL);
	if (FAILED(hr))
	{
		log::error << L"Create graphics failed; unable to create secondary surface (" << translateError(hr) << L")" << Endl;
		return false;
	}

	m_secondary->swap(m_ddsSecondary);

	return true;
}

Ref< ISurface > GraphicsSystemDdWm5::getPrimarySurface()
{
	return m_primary;
}

Ref< ISurface > GraphicsSystemDdWm5::getSecondarySurface()
{
	return m_secondary;
}

Ref< ISurface > GraphicsSystemDdWm5::createOffScreenSurface(const SurfaceDesc& surfaceDesc)
{
	return new SurfaceOffscreen(surfaceDesc);
}

void GraphicsSystemDdWm5::flip(bool waitVBlank)
{
	HRESULT hr;
	POINT offset = { 0, 0 };
	RECT rc;

	ClientToScreen(m_hWnd, &offset);
	GetClientRect(m_hWnd, &rc);
	OffsetRect(&rc, offset.x, offset.y);

	for (;;)
	{
		hr = m_ddsPrimary->Blt(&rc, m_ddsSecondary, NULL, 0, NULL);
		if (hr == DD_OK || hr != DDERR_WASSTILLDRAWING)
			break;
	}
}

	}
}
