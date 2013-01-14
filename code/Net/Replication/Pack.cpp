#include <limits>
#include "Core/Math/Const.h"
#include "Net/Replication/Pack.h"

namespace traktor
{
	namespace net
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
	float dx = u.x() / 128.0f;
	float dy = u.y() / 128.0f;
	float dz = u.z() / 128.0f;

	float md = std::numeric_limits< float >::max();
	for (int32_t i = 0; i < 128; ++i)
	{
		x += dx;
		y += dy;
		z += dz;

		int32_t ix = int32_t((x * 0.5f + 0.5f) * 255.0f);
		int32_t iy = int32_t((y * 0.5f + 0.5f) * 255.0f);
		int32_t iz = int32_t((z * 0.5f + 0.5f) * 255.0f);

		T_ASSERT (ix >= 0 && ix <= 255);
		T_ASSERT (iy >= 0 && iy <= 255);
		T_ASSERT (iz >= 0 && iz <= 255);

		Vector4 v(
			ix / 127.0f - 1.0f,
			iy / 127.0f - 1.0f,
			iz / 127.0f - 1.0f
		);

		float D = (u * v.length() - v).length();
		if (D < md)
		{
			out[0] = ix;
			out[1] = iy;
			out[2] = iz;
			md = D;
		}
	}

#if defined(_DEBUG)
	Vector4 check = unpackUnit(out);
	Vector4 error = (check - u).absolute();
	Scalar E = horizontalAdd4(error);
	T_ASSERT (E < 0.01f);
#endif
}

	}
}
