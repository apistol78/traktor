#include "Graphics/DdWm5/GraphicsSystemDdWm5.h"
#include "Graphics/DdWm5/SurfaceDdWm5.h"
#include "Core/Heap/HeapNew.h"
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

	switch (pixelFormat)
	{
	case PfeR5G5B5:
		ddpf.dwRBitMask = 0x00007C00;
		ddpf.dwGBitMask = 0x000003E0;
		ddpf.dwBBitMask = 0x0000001F;
		break;

	case PfeA1R5G5B5:
		//ddpf.dwFlags |= DDPF_ALPHAPIXELS;
		ddpf.dwRBitMask = 0x00007C00;
		ddpf.dwGBitMask = 0x000003E0;
		ddpf.dwBBitMask = 0x0000001F;
		//ddpf.dwRGBAlphaBitMask = 0x00008000;
		break;

	case PfeR5G6B5:
		ddpf.dwRBitMask = 0x0000F800;
		ddpf.dwGBitMask = 0x000007E0;
		ddpf.dwBBitMask = 0x0000001F;
		break;

	case PfeR8G8B8:
		ddpf.dwRBitMask = 0x00FF0000;
		ddpf.dwGBitMask = 0x0000FF00;
		ddpf.dwBBitMask = 0x000000FF;
		break;

	case PfeA8R8G8B8:
		//ddpf.dwFlags |= DDPF_ALPHAPIXELS;
		ddpf.dwRBitMask = 0x00FF0000;
		ddpf.dwGBitMask = 0x0000FF00;
		ddpf.dwBBitMask = 0x000000FF;
		//ddpf.dwRGBAlphaBitMask = 0xFF000000;
		break;

	default:
		return false;
	}

	return true;
}

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

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.graphics.GraphicsSystemDdWm5", GraphicsSystemDdWm5, GraphicsSystem)

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

bool GraphicsSystemDdWm5::create(const CreateDesc& createDesc)
{
	HRESULT hr;

	if (!(m_hWnd = (HWND)createDesc.windowHandle))
		return false;

	if (createDesc.fullScreen)
	{
		hr = m_dd->SetCooperativeLevel(m_hWnd, DDSCL_FULLSCREEN);
		if (FAILED(hr))
		{
			log::error << L"Create graphics failed, unable to set cooperative level (" << translateError(hr) << L")" << Endl;
			return false;
		}

		hr = m_dd->SetDisplayMode(
			createDesc.displayMode.width,
			createDesc.displayMode.height,
			createDesc.displayMode.bits,
			0,
			0
		);
		//if (FAILED(hr))
		//{
		//	log::error << L"Create graphics failed, unable to set display mode (" << translateError(hr) << L")" << Endl;
		//	return false;
		//}

		DDSURFACEDESC ddsd;

		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

		hr = m_dd->CreateSurface(&ddsd, &m_ddsPrimary.getAssign(), NULL);
		if (FAILED(hr))
		{
			log::error << L"Create graphics failed, unable to create primary surface (" << translateError(hr) << L")" << Endl;
			return false;
		}

		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
		ddsd.dwWidth = createDesc.displayMode.width;
		ddsd.dwHeight = createDesc.displayMode.height;
		getDDPixelFormat(createDesc.pixelFormat, ddsd.ddpfPixelFormat);

		hr = m_dd->CreateSurface(&ddsd, &m_ddsSecondary.getAssign(), NULL);
		if (FAILED(hr))
		{
			log::error << L"Create graphics failed, unable to create secondary surface (" << translateError(hr) << L")" << Endl;
			return false;
		}
	}
	else
	{
		hr = m_dd->SetCooperativeLevel(m_hWnd, DDSCL_NORMAL);
		if (FAILED(hr))
		{
			log::error << L"Create graphics failed, unable to set cooperative level (" << translateError(hr) << L")" << Endl;
			return false;
		}

		DDSURFACEDESC ddsd;

		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

		hr = m_dd->CreateSurface(&ddsd, &m_ddsPrimary.getAssign(), NULL);
		if (FAILED(hr))
		{
			log::error << L"Create graphics failed, unable to create primary surface (" << translateError(hr) << L")" << Endl;
			return false;
		}

		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
		ddsd.dwWidth = createDesc.displayMode.width;
		ddsd.dwHeight = createDesc.displayMode.height;
		getDDPixelFormat(createDesc.pixelFormat, ddsd.ddpfPixelFormat);

		hr = m_dd->CreateSurface(&ddsd, &m_ddsSecondary.getAssign(), NULL);
		if (FAILED(hr))
		{
			log::error << L"Create graphics failed, unable to create secondary surface (" << translateError(hr) << L")" << Endl;
			return false;
		}

		hr = m_dd->CreateClipper(0, &m_ddClipper.getAssign(), NULL);
		if (FAILED(hr))
		{
			log::error << L"Create graphics failed, unable to create clipper (" << translateError(hr) << L")" << Endl;
			return false;
		}

		hr = m_ddClipper->SetHWnd(0, m_hWnd);
		if (FAILED(hr))
		{
			log::error << L"Create graphics failed, unable to set window handle to clipper (" << translateError(hr) << L")" << Endl;
			return false;
		}

		hr = m_ddsPrimary->SetClipper(m_ddClipper);
		if (FAILED(hr))
		{
			log::error << L"Create graphics failed, unable to set clipper on primary surface (" << translateError(hr) << L")" << Endl;
			return false;
		}
	}

	m_primary = gc_new< SurfaceDdWm5 >(m_ddsPrimary);
	m_secondary = gc_new< SurfaceDdWm5 >(m_ddsSecondary);

	return true;
}

void GraphicsSystemDdWm5::destroy()
{
	m_ddClipper.release();
	m_ddsSecondary.release();
	m_ddsPrimary.release();
	m_dd.release();
}

bool GraphicsSystemDdWm5::resize(int width, int height)
{
	DDSURFACEDESC ddsd;
	HRESULT hr;

	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
	ddsd.dwWidth = width;
	ddsd.dwHeight = height;
	ddsd.ddpfPixelFormat = m_ddPixelFormat;

	ComRef< IDirectDrawSurface > ddsSecondary;
	hr = m_dd->CreateSurface(&ddsd, &ddsSecondary.getAssign(), NULL);
	if (FAILED(hr))
		return false;

	m_secondary->swap(ddsSecondary);
	m_ddsSecondary = ddsSecondary;

	return true;
}

Surface* GraphicsSystemDdWm5::getPrimarySurface()
{
	return m_primary;
}

Surface* GraphicsSystemDdWm5::getSecondarySurface()
{
	return m_secondary;
}

Surface* GraphicsSystemDdWm5::createOffScreenSurface(const SurfaceDesc& surfaceDesc)
{
	return 0;
}

void GraphicsSystemDdWm5::flip(bool waitVBlank)
{
	POINT pntTopLeft = { 0, 0 };
	RECT rcTarget;

	ClientToScreen(m_hWnd, &pntTopLeft);
	GetClientRect(m_hWnd, &rcTarget);
	OffsetRect(&rcTarget, pntTopLeft.x, pntTopLeft.y);

	for (;;)
	{
		HRESULT hr = m_ddsPrimary->Blt(&rcTarget, m_ddsSecondary, NULL, 0, NULL);
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
