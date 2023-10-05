/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <ctime>
#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Aabb2.h"
#include "Core/Math/Const.h"
#include "Core/Math/Format.h"
#include "Core/Math/Random.h"
#include "Core/Math/Triangulator.h"
#include "Core/Math/Winding2.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Shape/Editor/Bake/GBuffer.h"

namespace traktor::shape
{
	namespace
	{

class Barycentric
{
public:
	explicit Barycentric(
		const Vector2& v0,
		const Vector2& v1,
		const Vector2& v2
	)
	:	m_v0(v0)
	,	m_v1(v1)
	,	m_v2(v2)
	{
		m_denom = (v1.y - v2.y) * (v0.x - v2.x) + (v2.x - v1.x) * (v0.y - v2.y);
	}

	Vector4 factors(const Vector2& pt) const
	{
		const float alpha = ((m_v1.y - m_v2.y) * (pt.x - m_v2.x) + (m_v2.x - m_v1.x) * (pt.y - m_v2.y)) / m_denom;
		const float beta = ((m_v2.y - m_v0.y) * (pt.x - m_v2.x) + (m_v0.x - m_v2.x) * (pt.y - m_v2.y)) / m_denom;
		const float gamma = 1.0f - alpha - beta;
		return Vector4(alpha, beta, gamma, 0.0f);
	}

	bool inside(const Vector2& pt) const
	{
		const Vector4 f = factors(pt).xyz1();
		return compareAllGreaterEqual(f, Vector4::zero());
	}

private:
	Vector2 m_v0;
	Vector2 m_v1;
	Vector2 m_v2;
	float m_denom;
};

template < typename ValueType >
class Interpolants
{
public:
	explicit Interpolants(
		const ValueType& v0,
		const ValueType& v1,
		const ValueType& v2
	)
	:	m_v0(v0)
	,	m_v1(v1)
	,	m_v2(v2)
	{
	}

	ValueType evaluate(const Barycentric& bary, const Vector2& pt) const
	{
		const Vector4 f = bary.factors(pt);
		return m_v0 * f.x() + m_v1 * f.y() + m_v2 * f.z();
	}

private:
	ValueType m_v0;
	ValueType m_v1;
	ValueType m_v2;
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.GBuffer", GBuffer, Object)

bool GBuffer::create(int32_t width, int32_t height, const model::Model& model, const Transform& transform, uint32_t texCoordChannel)
{
	AlignedVector< Vector4 > positions;
	AlignedVector< Vector4 > normals;
	AlignedVector< Vector4 > tangents;
	Winding2 texCoords;
	Random rnd;

	m_width = width;
	m_height = height;
	m_data.resize(width * height);
	m_boundingBox = Aabb3();

	const Vector2 wh((float)width, (float)height);

	for (uint32_t i = 0; i < model.getPolygonCount(); ++i)
	{
		const auto& polygon = model.getPolygon(i);

		// Extract data for polygon.
		positions.resize(0);
		normals.resize(0);
		texCoords.resize(0);

		for (const auto index : polygon.getVertices())
		{
			const auto& vertex = model.getVertex(index);

			const uint32_t positionIndex = vertex.getPosition();
			positions.push_back(transform * model.getPosition(positionIndex).xyz1());

			const uint32_t normalIndex = vertex.getNormal();
			normals.push_back((transform * model.getNormal(normalIndex).xyz0()).normalized());

			const uint32_t texCoordIndex = vertex.getTexCoord(texCoordChannel);
			texCoords.push(model.getTexCoord(texCoordIndex) * wh - Vector2(0.5f, 0.5f));

			m_boundingBox.contain(positions.back());
		}

		// Triangulate winding so we can easily traverse lightmap fragments.
		Triangulator().freeze(texCoords.get(), Triangulator::Mode::Sorted, [&](size_t i0, size_t i1, size_t i2) {
			const Barycentric bary(
				texCoords[i0],
				texCoords[i1],
				texCoords[i2]
			);

			const Interpolants< Vector4 > ipolPositions(
				positions[i0],
				positions[i1],
				positions[i2]
			);

			const Interpolants< Vector4 > ipolNormals(
				normals[i0],
				normals[i1],
				normals[i2]
			);

			Aabb2 bbox;
			bbox.contain(texCoords[i0]);
			bbox.contain(texCoords[i1]);
			bbox.contain(texCoords[i2]);

			const float a = (texCoords[i1] - texCoords[i0]).length();
			const float b = (texCoords[i2] - texCoords[i1]).length();
			const float c = (texCoords[i0] - texCoords[i2]).length();
			const float s = 0.5f * (a + b + c);
			const float area = sqrt(s * (s - a) * (s - b) * (s - c));

			const int32_t pad = 2;
			const int32_t sx = (int32_t)(bbox.mn.x - pad - 0.5f);
			const int32_t ex = (int32_t)(bbox.mx.x + pad + 0.5f);
			const int32_t sy = (int32_t)(bbox.mn.y - pad - 0.5f);
			const int32_t ey = (int32_t)(bbox.mx.y + pad + 0.5f);

			for (int32_t x = sx; x <= ex; ++x)
			{
				for (int32_t y = sy; y <= ey; ++y)
				{
					if (x < 0 || x >= width || y < 0 || y >= height)
						continue;

					const float maxDistance = 1.0f;

					const Vector2 texelCenter = Vector2((float)x, (float)y);
					Vector2 pt = texelCenter;
					if (!bary.inside(pt))
					{
						const Vector2 cpt = texCoords.closest(pt);
						const Vector2 d = (pt - cpt).absolute();
						if (d.x > maxDistance || d.y > maxDistance)
							continue;
						pt = cpt;
					}

					const float distance = (pt - texelCenter).length();

					auto& e = m_data[x + y * m_width];
					if (e.polygon != ~0U)
					{
						if (distance > e.distance)
							continue;
						if (abs(distance - e.distance) < FUZZY_EPSILON && area < e.area)
							continue;
					}

					e.position = ipolPositions.evaluate(bary, pt).xyz1();
					e.normal = ipolNormals.evaluate(bary, pt).xyz0().normalized();
					e.polygon = i;
					e.material = polygon.getMaterial();
					e.area = area;
					e.distance = distance;
				}
			}
		});
	}

	return true;
}


}
