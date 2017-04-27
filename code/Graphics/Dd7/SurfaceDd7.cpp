/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Graphics/Dd7/SurfaceDd7.h"

namespace traktor
{
	namespace graphics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.graphics.SurfaceDd7", SurfaceDd7, ISurface)

SurfaceDd7::SurfaceDd7(IDirectDrawSurface7* dds)
:	m_dds(dds)
,	m_locked(false)
{
}

bool SurfaceDd7::getSurfaceDesc(SurfaceDesc& surfaceDesc) const
{
	DDSURFACEDESC2 ddsd;
	HRESULT hr;

	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	hr = m_dds->GetSurfaceDesc(&ddsd);
	if (FAILED(hr))
		return false;

	surfaceDesc.width = ddsd.dwWidth;
	surfaceDesc.height = ddsd.dwHeight;
	surfaceDesc.pitch = ddsd.lPitch;

	return true;
}

void* SurfaceDd7::lock(SurfaceDesc& surfaceDesc)
{
	DDSURFACEDESC2 ddsd;
	HRESULT hr;

	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	
	hr = m_dds->Lock(NULL, &ddsd, DDLOCK_WRITEONLY, NULL);
	if (FAILED(hr))
		return 0;

	m_locked = true;

	surfaceDesc.width = ddsd.dwWidth;
	surfaceDesc.height = ddsd.dwHeight;
	surfaceDesc.pitch = ddsd.lPitch;

	return ddsd.lpSurface;
}

void SurfaceDd7::unlock()
{
	T_ASSERT (m_locked);
	m_dds->Unlock(NULL);
	m_locked = false;
}

void SurfaceDd7::blt(
	ISurface* sourceSurface,
	int sourceX,
	int sourceY,
	int x,
	int y,
	int width,
	int height
)
{
	RECT rcDest = { x, y, x + width, y + height };
	RECT rcSource = { sourceX, sourceY, sourceX + width, sourceY + height };

#if defined(WINCE)

	m_dds->Blt(
		&rcDest,
		static_cast< SurfaceDd7* >(sourceSurface)->m_dds,
		&rcSource,
		0,
		NULL
	);

#else

	m_dds->BltFast(
		x,
		y,
		static_cast< SurfaceDd7* >(sourceSurface)->m_dds,
		&rcSource,
		DDBLTFAST_NOCOLORKEY
	);

#endif
}

void SurfaceDd7::swap(IDirectDrawSurface7* dds)
{
	T_ASSERT (!m_locked);
	m_dds = dds;
}

	}
}
