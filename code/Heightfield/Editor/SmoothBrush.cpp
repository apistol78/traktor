#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Color4f.h"
#include "Core/Math/Const.h"
#include "Drawing/Image.h"
#include "Heightfield/Editor/Convert.h"
#include "Heightfield/Editor/HeightfieldCompositor.h"
#include "Heightfield/Editor/HeightfieldLayer.h"
#include "Heightfield/Editor/SmoothBrush.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.hf.SmoothBrush", SmoothBrush, IBrush)

SmoothBrush::SmoothBrush(float radius)
:	m_radius(radius)
{
}

void SmoothBrush::apply(HeightfieldCompositor* compositor, const Vector4& at, Region& outDirty) const
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

	float cX = (maxX + minX) / 2.0f;
	float cZ = (maxZ + minZ) / 2.0f;

	const drawing::Image* baseImage = compositor->getBaseLayer()->getImage();
	const drawing::Image* offsetImage = compositor->getOffsetLayer()->getImage();
	drawing::Image* accumImage = compositor->getAccumLayer()->getImage();

	const height_t* baseHeights = static_cast< const height_t* >(baseImage->getData());
	const height_t* offsetHeights = static_cast< const height_t* >(offsetImage->getData());
	height_t* accumHeights = static_cast< height_t* >(accumImage->getData());

	for (int32_t iz = r.minZ; iz < r.maxZ; ++iz)
	{
		for (int32_t ix = r.minX; ix < r.maxX; ++ix)
		{
			float rx = (ix - cX) * 2.0f / (maxX - minX);
			float rz = (iz - cZ) * 2.0f / (maxZ - minZ);

			float d = clamp(1.0f - std::sqrt(rx * rx + rz * rz), 0.0f, 1.0f);
			if (d < FUZZY_EPSILON)
				continue;

			int32_t sh = 0;
			for (int32_t iiz = -1; iiz <= 1; ++iiz)
			{
				for (int32_t iix = -1; iix <= 1; ++iix)
				{
					uint32_t o = (ix + iix) + (iz + iiz) * size;
					sh += baseHeights[o] + unpackSignedHeight(offsetHeights[o]);
				}
			}
			sh /= 9;
			
			uint32_t o = ix + iz * size;

			int32_t ch = baseHeights[o] + unpackSignedHeight(offsetHeights[o]);
			int32_t ah = unpackSignedHeight(accumHeights[o]);
			int32_t bh = (sh - ch) / 2;

			accumHeights[o] = packSignedHeight(int32_t(ah * (1.0f - d) + bh * d));
		}
	}

	outDirty = r;
}

	}
}
