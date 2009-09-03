#include "Graphics/DdWm5/GraphicsSystemDdWm5.h"
#include "Graphics/DdWm5/SurfaceDdWm5.h"
#include "Graphics/DdWm5/SurfaceOffscreen.h"
#include "Core/Heap/GcNew.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace graphics
	{
		namespace
		{
//
//bool getDDPixelFormat(PixelFormatEnum pixelFormat, DDPIXELFORMAT& ddpf)
//{
//	memset(&ddpf, 0, sizeof(ddpf));
//	ddpf.dwSize = sizeof(ddpf);
//	ddpf.dwFlags = DDPF_RGB;
//	ddpf.dwRGBBitCount = getColorBits(pixelFormat);
//
//	switch (pixelFormat)
//	{
//	case PfeR5G5B5:
//		ddpf.dwRBitMask = 0x00007C00;
//		ddpf.dwGBitMask = 0x000003E0;
//		ddpf.dwBBitMask = 0x0000001F;
//		break;
//
//	case PfeA1R5G5B5:
//		ddpf.dwRBitMask = 0x00007C00;
//		ddpf.dwGBitMask = 0x000003E0;
//		ddpf.dwBBitMask = 0x0000001F;
//		break;
//
//	case PfeR5G6B5:
//		ddpf.dwRBitMask = 0x0000F800;
//		ddpf.dwGBitMask = 0x000007E0;
//		ddpf.dwBBitMask = 0x0000001F;
//		break;
//
//	case PfeR8G8B8:
//		ddpf.dwRBitMask = 0x00FF0000;
//		ddpf.dwGBitMask = 0x0000FF00;
//		ddpf.dwBBitMask = 0x000000FF;
//		break;
//
//	case PfeA8R8G8B8:
//		ddpf.dwRBitMask = 0x00FF0000;
//		ddpf.dwGBitMask = 0x0000FF00;
//		ddpf.dwBBitMask = 0x000000FF;
//		break;
//
//	default:
//		return false;
//	}
//
//	return true;
//}

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
//
//IDirectDrawSurface* createSecondarySurface(IDirectDraw* dd, DWORD dwFlags, DWORD dwCaps, PixelFormatEnum pixelFormat, DWORD dwWidth, DWORD dwHeight)
//{
//	IDirectDrawSurface* ddsSecondary;
//	DDSURFACEDESC ddsd;
//	HRESULT hr;
//
//	memset(&ddsd, 0, sizeof(ddsd));
//	ddsd.dwSize = sizeof(ddsd);
//	ddsd.dwFlags = dwFlags | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
//	ddsd.dwWidth = dwWidth;
//	ddsd.dwHeight = dwHeight;
//	ddsd.ddsCaps.dwCaps = dwCaps;
//
//	getDDPixelFormat(pixelFormat, ddsd.ddpfPixelFormat);
//
//	hr = dd->CreateSurface(&ddsd, &ddsSecondary, NULL);
//	if (FAILED(hr))
//		return 0;
//
//	return ddsSecondary;
//}

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

	if (!createDesc.fullScreen)
		return false;

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
		log::error << L"Create graphics failed; unable to set display mode (" << translateError(hr) << L")" << Endl;
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
		log::error << L"Create graphics failed; unable to create primary surface (" << translateError(hr) << L")" << Endl;
		return false;
	}

	m_primary = gc_new< SurfaceDdWm5 >(m_ddsPrimary);

	SurfaceDesc secondaryDesc;
	secondaryDesc.width = createDesc.displayMode.width;
	secondaryDesc.height = createDesc.displayMode.height;
	secondaryDesc.pitch = createDesc.displayMode.width * getColorBits(createDesc.pixelFormat) / 8;
	secondaryDesc.pixelFormat = createDesc.pixelFormat;

	m_secondary = gc_new< SurfaceOffscreen >(cref(secondaryDesc));

	return true;
}

void GraphicsSystemDdWm5::destroy()
{
	m_ddsPrimary.release();
	m_dd.release();
}

bool GraphicsSystemDdWm5::resize(int width, int height)
{
	return false;
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
	return gc_new< SurfaceOffscreen >(cref(surfaceDesc));
}

void GraphicsSystemDdWm5::flip(bool waitVBlank)
{
	DDSURFACEDESC ddsd;
	HRESULT hr;

	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	hr = m_ddsPrimary->Lock(NULL, &ddsd, DDLOCK_WRITEONLY, NULL);
	if (SUCCEEDED(hr))
	{
		SurfaceDesc desc;

		const uint8_t* s = static_cast< const uint8_t* >(m_secondary->lock(desc));
		uint8_t* d = static_cast< uint8_t* >(ddsd.lpSurface);

		switch (ddsd.ddpfPixelFormat.dwRGBBitCount)
		{
		case 16:
			for (uint32_t y = 0; y < ddsd.dwHeight; ++y)
			{
				const uint32_t* sr = reinterpret_cast< const uint32_t* >(s);
				uint16_t* dw = reinterpret_cast< uint16_t* >(d);

				for (uint32_t x = 0; x < ddsd.dwWidth; ++x)
				{
					uint8_t r = (sr[x] >> 16) & 255;
					uint8_t g = (sr[x] >> 8) & 255;
					uint8_t b = (sr[x]) & 255;

					dw[x] = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
				}

				s += desc.pitch;
				d += ddsd.lPitch;
			}
			break;

		case 24:
		case 32:
			break;
		}

		m_ddsPrimary->Unlock(NULL);
	}
}

	}
}
