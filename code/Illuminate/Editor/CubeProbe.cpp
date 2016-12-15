#include "Core/Math/MathUtils.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/MirrorFilter.h"
#include "Illuminate/Editor/CubeProbe.h"

namespace traktor
{
	namespace illuminate
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.illuminate.CubeProbe", CubeProbe, IProbe)

CubeProbe::CubeProbe(const drawing::Image* cubeMap)
{
	int32_t width = cubeMap->getWidth();
	int32_t height = cubeMap->getHeight();

	int32_t layout = 0;
	int32_t sideSize = height;

	if (height == width / 6)
	{
		// [+x][-x][+y][-y][+z][-z]
		layout = 0;
		sideSize = height;
	}
	else if (height / 3 == width / 4)
	{
		// [  ][+y][  ][  ]
		// [-x][+z][+x][-z]
		// [  ][-y][  ][  ]
		layout = 1;
		sideSize = height / 3;
	}
	else if (height / 4 == width / 3)
	{
		// [  ][+y][  ]
		// [-x][+z][+x]
		// [  ][-y][  ]
		// [  ][-z][  ]
		layout = 2;
		sideSize = height / 4;
	}

	for (int side = 0; side < 6; ++side)
	{
		m_side[side] = new drawing::Image(cubeMap->getPixelFormat(), sideSize, sideSize);

		if (layout == 0)
			m_side[side]->copy(cubeMap, side * sideSize, 0, sideSize, sideSize);
		else if (layout == 1)
		{
			const int32_t c_sideOffsets[][2] =
			{
				{ 2, 1 },
				{ 0, 1 },
				{ 1, 0 },
				{ 1, 2 },
				{ 1, 1 },
				{ 3, 1 }
			};
			m_side[side]->copy(
				cubeMap,
				c_sideOffsets[side][0] * sideSize,
				c_sideOffsets[side][1] * sideSize,
				sideSize,
				sideSize
			);
		}
		else if (layout == 2)
		{
			const int32_t c_sideOffsets[][2] =
			{
				{ 2, 1 },
				{ 0, 1 },
				{ 1, 0 },
				{ 1, 2 },
				{ 1, 1 },
				{ 1, 3 }
			};
			m_side[side]->copy(
				cubeMap,
				c_sideOffsets[side][0] * sideSize,
				c_sideOffsets[side][1] * sideSize,
				sideSize,
				sideSize
			);
			if (side == 5)
			{
				// Flip -Z as it's defined up-side down in this layout.
				drawing::MirrorFilter filter(true, true);
				m_side[side]->apply(&filter);
			}
		}
	}
}

Color4f CubeProbe::sample(const Vector4& direction) const
{
	int32_t major = majorAxis3(direction);
	int32_t side = major * 2 + (direction[major] >= 0.0f ? 0 : 1);

	float u, v, n;
	switch (major)
	{
	case 0:
		u = -sign(direction.x()) * direction.z();
		v = -direction.y();
		n = 1.0f / abs(direction.x());
		break;

	case 1:
		u = direction.x();
		v = sign(direction.y()) * direction.z();
		n = 1.0f / abs(direction.y());
		break;

	case 2:
		u = sign(direction.z()) * direction.x();
		v = -direction.y();
		n = 1.0f / abs(direction.z());
		break;
	}

	u *= n;
	v *= n;

	int32_t size = m_side[side]->getHeight();

	int32_t x = clamp< int32_t >((u * 0.5f + 0.5f) * size, 0, size);
	int32_t y = clamp< int32_t >((v * 0.5f + 0.5f) * size, 0, size);

	Color4f s;
	m_side[side]->getPixel(x, y, s);

	return s;
}

	}
}
