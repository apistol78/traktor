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

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.graphics.GraphicsSystemDdWm5", GraphicsSystemDdWm5, GraphicsSystem)

GraphicsSystemDdWm5::GraphicsSystemDdWm5()
:	m_hWnd(NULL)
,	m_supportHardwareFlip(false)
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
	DDCAPS ddc;
	DDSURFACEDESC ddsd;
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
		log::warning << L"Create graphics failed; unable to set display mode (" << translateError(hr) << L")" << Endl;

	std::memset(&ddc, 0, sizeof(ddc));
	ddc.dwSize = sizeof(ddc);
	m_dd->GetCaps(&ddc, NULL);

	//m_supportHardwareFlip = (ddc.ddsCaps.dwCaps & DDSCAPS_BACKBUFFER) != 0;
	//m_supportHardwareFlip &= (ddc.ddsCaps.dwCaps & DDSCAPS_FLIP) != 0;
	//if (!m_supportHardwareFlip)
	//	log::warning << L"Doesn't support hardware flip; performance might be poor" << Endl;

	m_supportHardwareFlip = false;

	if (m_supportHardwareFlip)
	{
		std::memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP;
		ddsd.dwBackBufferCount = 1;

		hr = m_dd->CreateSurface(&ddsd, &m_ddsPrimary.getAssign(), NULL);
		if (FAILED(hr))
		{
			log::error << L"Create graphics failed; unable to create primary surface (" << translateError(hr) << L")" << Endl;
			return false;
		}

		GetBackSurfaceEnum callback;
		m_ddsPrimary->EnumAttachedSurfaces(&callback, &GetBackSurfaceEnum::callback);
		m_ddsSecondary = callback.m_ddBackSurface;
	}
	else
	{
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
		ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT;
		ddsd.dwWidth = createDesc.displayMode.width;
		ddsd.dwHeight = createDesc.displayMode.height;

		hr = m_dd->CreateSurface(&ddsd, &m_ddsSecondary.getAssign(), NULL);
		if (FAILED(hr))
		{
			log::error << L"Create graphics failed; unable to create secondary surface (" << translateError(hr) << L")" << Endl;
			return false;
		}
	}

	m_primary = gc_new< SurfaceDdWm5 >(m_ddsPrimary);
	m_secondary = gc_new< SurfaceDdWm5 >(m_ddsSecondary);
	return true;
}

void GraphicsSystemDdWm5::destroy()
{
	m_ddsPrimary.release();
	m_ddsSecondary.release();
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
	if (m_supportHardwareFlip)
		m_ddsPrimary->Flip(NULL, DDFLIP_WAITNOTBUSY);
	else
		m_ddsPrimary->Blt(NULL, m_ddsSecondary, NULL, DDBLT_WAITNOTBUSY, 0);
}

	}
}
