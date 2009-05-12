#include "Graphics/DdWm5/SurfaceDdWm5.h"

namespace traktor
{
	namespace graphics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.graphics.SurfaceDdWm5", SurfaceDdWm5, Surface)

SurfaceDdWm5::SurfaceDdWm5(IDirectDrawSurface* dds)
:	m_dds(dds)
,	m_locked(false)
{
}

bool SurfaceDdWm5::getSurfaceDesc(SurfaceDesc& surfaceDesc) const
{
	DDSURFACEDESC ddsd;
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

void* SurfaceDdWm5::lock(SurfaceDesc& surfaceDesc)
{
	DDSURFACEDESC ddsd;
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

void SurfaceDdWm5::unlock()
{
	T_ASSERT (m_locked);
	m_dds->Unlock(NULL);
	m_locked = false;
}

void SurfaceDdWm5::blt(
	Surface* sourceSurface,
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

	m_dds->Blt(
		&rcDest,
		static_cast< SurfaceDdWm5* >(sourceSurface)->m_dds,
		&rcSource,
		0,
		NULL
	);
}

void SurfaceDdWm5::swap(IDirectDrawSurface* dds)
{
	T_ASSERT (!m_locked);
	m_dds = dds;
}

	}
}
