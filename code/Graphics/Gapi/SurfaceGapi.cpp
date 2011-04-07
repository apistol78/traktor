#include "Graphics/Gapi/SurfaceGapi.h"

namespace traktor
{
	namespace graphics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.graphics.SurfaceGapi", SurfaceGapi, Surface)

SurfaceGapi::SurfaceGapi(const SurfaceDesc& surfaceDesc)
:	m_surfaceDesc(surfaceDesc)
{
	m_surfaceDesc.pitch = m_surfaceDesc.width * getByteSize(m_surfaceDesc.pixelFormat);
	m_data.resize(m_surfaceDesc.height * m_surfaceDesc.pitch);
}

bool SurfaceGapi::getSurfaceDesc(SurfaceDesc& surfaceDesc) const
{
	surfaceDesc = m_surfaceDesc;
	return true;
}

void* SurfaceGapi::lock(SurfaceDesc& surfaceDesc)
{
	surfaceDesc = m_surfaceDesc;
	return &m_data[0];
}

void SurfaceGapi::unlock()
{
}

void SurfaceGapi::blt(
	Surface* sourceSurface,
	int sourceX,
	int sourceY,
	int x,
	int y,
	int width,
	int height
)
{
	const SurfaceDesc& sourceDesc = checked_type_cast< SurfaceGapi* >(sourceSurface)->m_surfaceDesc;

	unsigned char* w = &m_data[x + y * m_surfaceDesc.pitch];
	unsigned char* r = &checked_type_cast< SurfaceGapi* >(sourceSurface)->m_data[sourceX + y * sourceDesc.pitch];

	for (int iy = 0; iy < height; ++iy)
	{
		unsigned char* wr = w;
		unsigned char* rr = r;

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
