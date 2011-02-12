#include "Physics/Heightfield.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Reader.h"
#include "Core/Io/Writer.h"
#include "Core/Math/Float.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.Heightfield", Heightfield, Object)

Heightfield::Heightfield(uint32_t size, const Vector4& worldExtent)
:	m_size(size)
,	m_worldExtent(worldExtent)
,	m_heights(new float [size * size])
{
}

Heightfield::~Heightfield()
{
	delete[] m_heights;
}

float Heightfield::getSampleBilinear(float x, float z) const
{
	float hx = m_size * (x + m_worldExtent.x() * 0.5f) / m_worldExtent.x() - 0.5f;
	float hz = m_size * (z + m_worldExtent.z() * 0.5f) / m_worldExtent.z() - 0.5f;

	int ix1 = int(std::floor(hx));
	int iz1 = int(std::floor(hz));

	int32_t size = int32_t(m_size - 1);

	if (ix1 < 0)
		ix1 = 0;
	else if (ix1 >= size - 1)
		ix1 = size - 2;

	if (iz1 < 0)
		iz1 = 0;
	else if (iz1 >= size - 1)
		iz1 = size - 2;

	int ix2 = ix1 + 1;
	int iz2 = iz1 + 1;

	float hc[] =
	{
		m_heights[(ix1 & size) + (iz1 & size) * m_size],
		m_heights[(ix2 & size) + (iz1 & size) * m_size],
		m_heights[(ix1 & size) + (iz2 & size) * m_size],
		m_heights[(ix2 & size) + (iz2 & size) * m_size]
	};

	float fx = hx - ix1;
	float fz = hz - iz1;

	float h1 = hc[0] + (hc[1] - hc[0]) * fx;
	float h2 = hc[2] + (hc[3] - hc[2]) * fx;
	float h = h1 + (h2 - h1) * fz;

	return h * m_worldExtent.y() * 0.5f;
}

float Heightfield::getSampleTrilinear(float x, float z) const
{
	float wsx = float(m_worldExtent.x()) / (m_size - 1);
	float wsz = float(m_worldExtent.z()) / (m_size - 1);

	float h =
		getSampleBilinear(x - wsx, z - wsz) +
		getSampleBilinear(x + wsx, z - wsz) +
		getSampleBilinear(x + wsx, z + wsz) +
		getSampleBilinear(x - wsx, z + wsz);

	return h * 0.25f;
}

bool Heightfield::read(IStream* stream)
{
	Reader reader(stream);
	reader.read(m_heights, m_size * m_size, sizeof(float));
	return true;
}

bool Heightfield::write(IStream* stream)
{
	Writer writer(stream);
	writer.write(m_heights, m_size * m_size, sizeof(float));
	return true;
}

	}
}
