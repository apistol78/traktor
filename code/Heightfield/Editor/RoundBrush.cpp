#include "Core/Math/Color4f.h"
#include "Core/Math/Const.h"
#include "Drawing/Image.h"
#include "Heightfield/Editor/HeightfieldCompositor.h"
#include "Heightfield/Editor/HeightfieldLayer.h"
#include "Heightfield/Editor/RoundBrush.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.hf.RoundBrush", RoundBrush, IBrush)

RoundBrush::RoundBrush(float radius, float pressure)
:	m_radius(radius)
,	m_pressure(pressure)
{
}

void RoundBrush::apply(HeightfieldCompositor* compositor, const Vector4& at, float is) const
{
	const Vector4& worldExtent = compositor->getWorldExtent();

	float y = 1.0f / worldExtent.y();

	float cx = at.x();
	float cz = at.z();

	Color4f offset;

	float minX = compositor->worldToGridX(cx - m_radius);
	float maxX = compositor->worldToGridX(cx + m_radius);
	float minZ = compositor->worldToGridZ(cz - m_radius);
	float maxZ = compositor->worldToGridZ(cz + m_radius);

	int32_t iminX = int32_t(floor(minX));
	int32_t imaxX = int32_t(ceil(maxX));
	int32_t iminZ = int32_t(floor(minZ));
	int32_t imaxZ = int32_t(ceil(maxZ));

	float cX = (maxX + minX) / 2.0f;
	float cZ = (maxZ + minZ) / 2.0f;

	drawing::Image* offsetImage = compositor->getOffsetLayer()->getImage();

	for (int32_t iz = iminZ; iz <= imaxZ; ++iz)
	{
		for (int32_t ix = iminX; ix <= imaxX; ++ix)
		{
			float rx = (ix - cX) * 2.0f / (maxX - minX);
			float rz = (iz - cZ) * 2.0f / (maxZ - minZ);

			float r = 1.0f - clamp(sqrt(rx * rx + rz * rz), 0.0f, 1.0f);
			float c = sin(r * HALF_PI);

			offsetImage->getPixel(ix, iz, offset);
			offset += Color4f(y * c * is * m_pressure, 0.0f, 0.0f, 0.0f);
			offsetImage->setPixel(ix, iz, offset);
		}
	}

	compositor->updateMergedLayer(iminX, imaxX, iminZ, imaxZ);
}

	}
}
