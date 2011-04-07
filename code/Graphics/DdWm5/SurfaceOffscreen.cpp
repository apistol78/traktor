#include "Graphics/DdWm5/SurfaceOffscreen.h"

namespace traktor
{
	namespace graphics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.graphics.SurfaceOffscreen", SurfaceOffscreen, ISurface)

SurfaceOffscreen::SurfaceOffscreen(const SurfaceDesc& surfaceDesc)
:	m_surfaceDesc(surfaceDesc)
{
	m_surfaceDesc.pitch = m_surfaceDesc.width * getByteSize(m_surfaceDesc.pixelFormat);
	m_data.reset(new uint8_t [m_surfaceDesc.height * m_surfaceDesc.pitch]);
}

bool SurfaceOffscreen::getSurfaceDesc(SurfaceDesc& surfaceDesc) const
{
	surfaceDesc = m_surfaceDesc;
	return true;
}

void* SurfaceOffscreen::lock(SurfaceDesc& surfaceDesc)
{
	surfaceDesc = m_surfaceDesc;
	return m_data.ptr();
}

void SurfaceOffscreen::unlock()
{
}

void SurfaceOffscreen::blt(
	ISurface* sourceSurface,
	int sourceX,
	int sourceY,
	int x,
	int y,
	int width,
	int height
)
{
	const SurfaceDesc& sourceDesc = checked_type_cast< SurfaceOffscreen* >(sourceSurface)->m_surfaceDesc;

	uint8_t* w = &m_data[x + y * m_surfaceDesc.pitch];
	uint8_t* r = &checked_type_cast< SurfaceOffscreen* >(sourceSurface)->m_data[sourceX + y * sourceDesc.pitch];

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

	}
}
