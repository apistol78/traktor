#include "Core/Math/Aabb2.h"
#include "Core/Math/Triangulator.h"
#include "Core/Math/Winding2.h"
#include "Illuminate/Editor/GBuffer.h"

namespace traktor
{
	namespace illuminate
	{
		namespace
		{

const int32_t c_dilateOffsets[][2] =
{
	{ -1,  0 },
	{  1,  0 },
	{  0, -1 },
	{  0,  1 }
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.illuminate.GBuffer", GBuffer, Object)

GBuffer::GBuffer()
:	m_width(0)
,	m_height(0)
{
}

bool GBuffer::create(const AlignedVector< Surface >& surfaces, int32_t width, int32_t height)
{
	std::vector< Triangulator::Triangle > triangles;
	const Vector2 dim(width, height);
	const Vector2 half(0.5f, 0.5f);
	Vector4 P[3], N[3];
	Winding2 t;

	t.points.resize(3);

	m_width = width;
	m_height = height;

	AlignedVector< Element > data(width * height);

	// Rasterize each surface into gbuffer.
	for (uint32_t j = 0; j < surfaces.size(); ++j)
	{
		const Surface& s = surfaces[j];

		AlignedVector< Vector2 > texCoords(s.count);
		for (int32_t k = 0; k < s.count; ++k)
			texCoords[k] = s.texCoords[k] * dim - half;

		triangles.resize(0);
		Triangulator().freeze(texCoords, triangles);

		for (std::vector< Triangulator::Triangle >::const_iterator k = triangles.begin(); k != triangles.end(); ++k)
		{
			size_t i0 = k->indices[0];
			size_t i1 = k->indices[1];
			size_t i2 = k->indices[2];

			t.points[0] = texCoords[i0];
			t.points[1] = texCoords[i1];
			t.points[2] = texCoords[i2];

			P[0] = s.points[i0];
			P[1] = s.points[i1];
			P[2] = s.points[i2];

			N[0] = s.normals[i0];
			N[1] = s.normals[i1];
			N[2] = s.normals[i2];

			Aabb2 aabb;
			aabb.contain(t.points[0]);
			aabb.contain(t.points[1]);
			aabb.contain(t.points[2]);

			int32_t x0 = int32_t(aabb.mn.x);
			int32_t x1 = int32_t(aabb.mx.x + 1);
			int32_t y0 = int32_t(aabb.mn.y);
			int32_t y1 = int32_t(aabb.mx.y + 1);

			float denom = (t.points[1].y - t.points[2].y) * (t.points[0].x - t.points[2].x) + (t.points[2].x - t.points[1].x) * (t.points[0].y - t.points[2].y);
			float invDenom = 1.0f / denom;

			for (int32_t y = y0; y <= y1; ++y)
			{
				for (int32_t x = x0; x <= x1; ++x)
				{
					Vector2 pt = Vector2(x + 0.5f, y + 0.5f);

					float alpha = ((t.points[1].y - t.points[2].y) * (pt.x - t.points[2].x) + (t.points[2].x - t.points[1].x) * (pt.y - t.points[2].y)) * invDenom;
					float beta = ((t.points[2].y - t.points[0].y) * (pt.x - t.points[2].x) + (t.points[0].x - t.points[2].x) * (pt.y - t.points[2].y)) * invDenom;
					float gamma = 1.0f - alpha - beta;

					if (alpha >= 0.0f && beta >= 0.0f && gamma >= 0.0f)
					{
						if (x >= 0 && y >= 0 && x < width && y < height)
						{
							Element& e = data[x + y * width];
							if (e.surfaceIndex < 0)
							{
								Vector4 position = (P[0] * Scalar(alpha) + P[1] * Scalar(beta) + P[2] * Scalar(gamma)).xyz1();
								Vector4 normal = (N[0] * Scalar(alpha) + N[1] * Scalar(beta) + N[2] * Scalar(gamma)).xyz0().normalized();
								e.surfaceIndex = j;
								e.position = position;
								e.normal = normal;
							}
						}
					}
				}
			}
		}
	}

	m_data.resize(width * height);

	Element* from = data.ptr();
	Element* to = m_data.ptr();

	for (int32_t iter = 0; iter < 9; ++iter)
	{
		for (int32_t y = 0; y < height; ++y)
		{
			for (int32_t x = 0; x < width; ++x)
			{
				if (from[x + y * width].surfaceIndex >= 0)
				{
					to[x + y * width] = from[x + y * width];
					continue;
				}
				for (int32_t i = 0; i < sizeof_array(c_dilateOffsets); ++i)
				{
					int32_t sx = x + c_dilateOffsets[i][0];
					int32_t sy = y + c_dilateOffsets[i][1];
					if (sx >= 0 && sx < width && sy >= 0 && sy < height)
					{
						if (from[sx + sy * width].surfaceIndex >= 0)
						{
							to[x + y * width] = from[sx + sy * width];
							break;
						}
					}
				}
			}
		}

		std::swap(from, to);
	}
	T_FATAL_ASSERT(from == m_data.ptr());

	return true;
}

	}
}
