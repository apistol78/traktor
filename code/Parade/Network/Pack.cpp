#include <limits>
#include "Core/Math/Const.h"
#include "Parade/Network/Pack.h"

namespace traktor
{
	namespace parade
	{

Vector4 unpackUnit(const uint8_t u[3])
{
	Vector4 v(
		u[0] / 127.0f - 1.0f,
		u[1] / 127.0f - 1.0f,
		u[2] / 127.0f - 1.0f
	);

	Scalar ln = v.length();
	if (ln > FUZZY_EPSILON)
		return v / ln;
	else
		return Vector4::zero();
}

void packUnit(const Vector4& u, uint8_t out[3])
{
	float x = 0.0f, y = 0.0f, z = 0.0f;
	float dx = u.x() / 8.0f;
	float dy = u.y() / 8.0f;
	float dz = u.z() / 8.0f;

	float md = std::numeric_limits< float >::max();
	for (int32_t i = 0; i < 8; ++i)
	{
		x += dx;
		y += dy;
		z += dz;

		int32_t ix = int32_t((x * 0.5f + 0.5f) * 255.0f);
		int32_t iy = int32_t((y * 0.5f + 0.5f) * 255.0f);
		int32_t iz = int32_t((z * 0.5f + 0.5f) * 255.0f);

		for (int32_t dx = -1; dx <= 1; ++dx)
		{
			int32_t iix = ix + dx;
			if (iix < 0 || iix > 255)
				continue;

			for (int32_t dy = -1; dy <= 1; ++dy)
			{
				int32_t iiy = iy + dy;
				if (iiy < 0 || iiy > 255)
					continue;

				for (int32_t dz = -1; dz <= 1; ++dz)
				{
					int32_t iiz = iz + dz;
					if (iiz < 0 || iiz > 255)
						continue;

					Vector4 v(
						iix / 127.0f - 1.0f,
						iiy / 127.0f - 1.0f,
						iiz / 127.0f - 1.0f
					);

					float D = (u * v.length() - v).length();
					if (D < md)
					{
						out[0] = iix;
						out[1] = iiy;
						out[2] = iiz;
						md = D;
					}
				}
			}
		}
	}
}

	}
}
