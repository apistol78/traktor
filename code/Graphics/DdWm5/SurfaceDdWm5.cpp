#include "Core/Misc/TString.h"
#include "Graphics/DdWm5/SurfaceDdWm5.h"

namespace traktor
{
	namespace graphics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.graphics.SurfaceDdWm5", SurfaceDdWm5, ISurface)

SurfaceDdWm5::SurfaceDdWm5(IDirectDrawSurface* dds)
:	m_dds(dds)
,	m_hFont(0)
,	m_locked(false)
{
}

SurfaceDdWm5::~SurfaceDdWm5()
{
	if (m_hFont != 0)
		DeleteObject(m_hFont);
}

bool SurfaceDdWm5::getSurfaceDesc(SurfaceDesc& surfaceDesc) const
{
	DDSURFACEDESC ddsd;
	HRESULT hr;

	std::memset(&ddsd, 0, sizeof(ddsd));
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

	std::memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	
	hr = m_dds->Lock(NULL, &ddsd, DDLOCK_DISCARD | DDLOCK_WRITEONLY, NULL);
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

	m_dds->Blt(
		&rcDest,
		static_cast< SurfaceDdWm5* >(sourceSurface)->m_dds,
		&rcSource,
		0,
		NULL
	);
}

void SurfaceDdWm5::setFontSize(int32_t size)
{
	HFONT hCurrentFont = 0;

	if (m_hFont)
		hCurrentFont = m_hFont;
	else
	{
		HDC hDC;
		m_dds->GetDC(&hDC);
		hCurrentFont = (HFONT)GetCurrentObject(hDC, OBJ_FONT);
		m_dds->ReleaseDC(hDC);
	}

	LOGFONT lf;
	std::memset(&lf, 0, sizeof(lf));
	GetObject(hCurrentFont, sizeof(lf), &lf);
	lf.lfHeight = size;

	if (m_hFont != 0)
		DeleteObject(m_hFont);

	m_hFont = CreateFontIndirect(&lf);
}

void SurfaceDdWm5::getTextExtent(const std::wstring& text, int32_t& outWidth, int32_t& outHeight)
{
	SIZE size;
	HDC hDC;

	m_dds->GetDC(&hDC);
	HFONT hOldFont = (HFONT)SelectObject(hDC, m_hFont);

	GetTextExtentPoint32(hDC, wstots(text).c_str(), int(text.length()), &size);

	SelectObject(hDC, hOldFont);
	m_dds->ReleaseDC(hDC);

	outWidth = size.cx;
	outHeight = size.cy;
}

void SurfaceDdWm5::drawText(
	int32_t x,
	int32_t y,
	const std::wstring& text,
	const Color4ub& color
)
{
	RECT wrc = { x, y, 0, 0 };
	UINT format = DT_SINGLELINE | DT_NOCLIP;
	HDC hDC;

	m_dds->GetDC(&hDC);

	SetBkMode(hDC, TRANSPARENT);
	HFONT hOldFont = (HFONT)SelectObject(hDC, m_hFont);

	SetTextColor(hDC, RGB(color.r, color.g, color.b));
	DrawText(hDC, wstots(text).c_str(), int(text.length()), &wrc, format);

	SelectObject(hDC, hOldFont);
	m_dds->ReleaseDC(hDC);
}

void SurfaceDdWm5::swap(IDirectDrawSurface* dds)
{
	T_ASSERT (!m_locked);
	m_dds = dds;
}

	}
}
