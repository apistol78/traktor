#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Color4f.h"
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Drawing/Image.h"
#include "Heightfield/Editor/Convert.h"
#include "Heightfield/Editor/HeightfieldCompositor.h"
#include "Heightfield/Editor/HeightfieldLayer.h"
#include "Heightfield/Editor/RoundBrush.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.hf.RoundBrush", RoundBrush, IBrush)

RoundBrush::RoundBrush(float radius, float height)
:	m_radius(radius)
,	m_height(height)
{
}

void RoundBrush::apply(HeightfieldCompositor* compositor, const Vector4& at, Region& outDirty) const
{
	const Vector4& worldExtent = compositor->getWorldExtent();

	float cx = at.x();
	float cz = at.z();

	float minX = compositor->worldToGridX(cx - m_radius);
	float maxX = compositor->worldToGridX(cx + m_radius);
	float minZ = compositor->worldToGridZ(cz - m_radius);
	float maxZ = compositor->worldToGridZ(cz + m_radius);

	Region r(
		int32_t(floor(minX)),
		int32_t(floor(minZ)),
		int32_t(ceil(maxX)),
		int32_t(ceil(maxZ))
	);
	
	int32_t size = compositor->getSize();

	r.intersect(Region(0, 0, size, size));
	if (r.empty())
		return;

	float y = m_height / worldExtent.y();

	float cX = (maxX + minX) / 2.0f;
	float cZ = (maxZ + minZ) / 2.0f;

	drawing::Image* accumImage = compositor->getAccumLayer()->getImage();
	height_t* accumHeights = static_cast< height_t* >(accumImage->getData());

	for (int32_t iz = r.minZ; iz < r.maxZ; ++iz)
	{
		for (int32_t ix = r.minX; ix < r.maxX; ++ix)
		{
			float rx = (ix - cX) * 2.0f / (maxX - minX);
			float rz = (iz - cZ) * 2.0f / (maxZ - minZ);

			float r = 1.0f - clamp(float(std::sqrt(rx * rx + rz * rz)), 0.0f, 1.0f);
			float c = sin(r * HALF_PI);

			height_t v = packSignedHeight(int32_t(y * c * 65535));
			height_t& accum = accumHeights[ix + iz * size];
			if (
				(y > 0.0f && accum < v) ||
				(y < 0.0f && accum > v)
			)
				accum = v;
		}
	}

	outDirty = r;
}

	}
}
