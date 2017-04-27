/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Graphics/Gdi/SurfaceGdi.h"

namespace traktor
{
	namespace graphics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.graphics.SurfaceGdi", SurfaceGdi, ISurface)

SurfaceGdi::SurfaceGdi(const SurfaceDesc& surfaceDesc)
:	m_surfaceDesc(surfaceDesc)
{
	m_surfaceDesc.pitch = m_surfaceDesc.width * getByteSize(m_surfaceDesc.pixelFormat);
	m_data.resize(m_surfaceDesc.height * m_surfaceDesc.pitch);
}

bool SurfaceGdi::getSurfaceDesc(SurfaceDesc& surfaceDesc) const
{
	surfaceDesc = m_surfaceDesc;
	return true;
}

void* SurfaceGdi::lock(SurfaceDesc& surfaceDesc)
{
	surfaceDesc = m_surfaceDesc;
	return &m_data[0];
}

void SurfaceGdi::unlock()
{
}

void SurfaceGdi::blt(
	ISurface* sourceSurface,
	int sourceX,
	int sourceY,
	int x,
	int y,
	int width,
	int height
)
{
	const SurfaceDesc& sourceDesc = checked_type_cast< SurfaceGdi*, false >(sourceSurface)->m_surfaceDesc;

	uint8_t* w = &m_data[x + y * m_surfaceDesc.pitch];
	uint8_t* r = &checked_type_cast< SurfaceGdi*, false >(sourceSurface)->m_data[sourceX + y * sourceDesc.pitch];

	for (int iy = 0; iy < height; ++iy)
	{
		uint8_t* wr = w;
		uint8_t* rr = r;

		for (int ix = 0; ix < width; ++ix)
		{
			convertPixel(
				m_surfaceDesc.pixelFormat,
				wr,
				sourceDesc.pixelFormat,
				rr
			);

			wr += getByteSize(m_surfaceDesc.pixelFormat);
			rr += getByteSize(sourceDesc.pixelFormat);
		}

		w += m_surfaceDesc.pitch;
		r += sourceDesc.pitch;
	}
}

void SurfaceGdi::setFontSize(int32_t size)
{
}

void SurfaceGdi::getTextExtent(const std::wstring& text, int32_t& outWidth, int32_t& outHeight)
{
}

void SurfaceGdi::drawText(
	int32_t x,
	int32_t y,
	const std::wstring& text,
	const Color4ub& color
)
{
}

	}
}

