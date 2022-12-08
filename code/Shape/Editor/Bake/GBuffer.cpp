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

namespace traktor
{
	namespace shape
	{
		namespace
		{

class Barycentric
{
public:
	Barycentric(
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
	Interpolants(
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

	m_width = width;
	m_height = height;
	m_data.resize(width * height);
	m_boundingBox = Aabb3();

	for (int32_t i = 0; i < width * height; ++i)
	{
		m_data[i].position = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		m_data[i].normal = Vector4(0.0f, 1.0f, 0.0f, 0.0f);
		m_data[i].tangent = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
		m_data[i].polygon = model::c_InvalidIndex;
		m_data[i].material = 0;
		m_data[i].delta = 0.0f;
		m_data[i].distance = 0.0f;
	}

	for (uint32_t i = 0; i < model.getPolygonCount(); ++i)
	{
		const auto& polygon = model.getPolygon(i);

		// Extract data for polygon.
		positions.resize(0);
		normals.resize(0);
		tangents.resize(0);
		texCoords.resize(0);
		Aabb2 texBounds;

		for (const auto index : polygon.getVertices())
		{
			const auto& vertex = model.getVertex(index);

			const uint32_t positionIndex = vertex.getPosition();
			positions.push_back(transform * model.getPosition(positionIndex).xyz1());

			const uint32_t normalIndex = vertex.getNormal();
			normals.push_back((transform * model.getNormal(normalIndex).xyz0()).normalized());

			const uint32_t tangentIndex = vertex.getTangent();
			tangents.push_back((transform * model.getNormal(tangentIndex).xyz0()).normalized());

			const uint32_t texCoordIndex = vertex.getTexCoord(texCoordChannel);
			texCoords.push(
				model.getTexCoord(texCoordIndex) * Vector2((float)width, (float)height) - Vector2(0.5f, 0.5f)
			);
			texBounds.contain(texCoords.get().back());

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

			const Interpolants< Vector4 > ipolTangents(
				tangents[i0],
				tangents[i1],
				tangents[i2]
			);

			Aabb2 bbox;
			bbox.contain(texCoords[i0]);
			bbox.contain(texCoords[i1]);
			bbox.contain(texCoords[i2]);

			const int32_t sx = (int32_t)(bbox.mn.x - 4);
			const int32_t ex = (int32_t)(bbox.mx.x + 4);
			const int32_t sy = (int32_t)(bbox.mn.y - 4);
			const int32_t ey = (int32_t)(bbox.mx.y + 4);

			for (int32_t x = sx; x <= ex; ++x)
			{
				for (int32_t y = sy; y <= ey; ++y)
				{
					if (x < 0 || x >= width || y < 0 || y >= height)
						continue;

					const Vector2 pt((float)x, (float)y);

					float distance = std::numeric_limits< float >::max();
					if (bary.inside(pt))
						distance = 0.0f;
					else
					{
						const Vector2 cpt = texCoords.closest(pt);
						const float fd = (pt - cpt).length();
						if (fd < 4.0f)
							distance = fd + 1.0f;
						else
							continue;
					}

					Element& elm = m_data[x + y * m_width];

					// Do not overwrite existing data unless we're closer to be inside triangle.
					if (elm.polygon != model::c_InvalidIndex)
					{
						if (elm.distance < distance)
							continue;
					}

					elm.position = ipolPositions.evaluate(bary, pt).xyz1();
					elm.normal = ipolNormals.evaluate(bary, pt).xyz0().normalized();
					elm.tangent = ipolTangents.evaluate(bary, pt).xyz0().normalized();
					elm.polygon = i;
					elm.material = polygon.getMaterial();
					elm.distance = distance;

					// Evaluate delta magnitude of position in world space per texel offset.
					const Vector4 ddx = ipolPositions.evaluate(bary, pt + Vector2(1.0f, 0.0f)).xyz1() - elm.position;
					const Vector4 ddy = ipolPositions.evaluate(bary, pt + Vector2(0.0f, 1.0f)).xyz1() - elm.position;
					const Vector4 duv = max(ddx.absolute(), ddy.absolute());
					elm.delta = max(max(duv.x(), duv.y()), duv.z()) * Scalar(sqrt(2.0f));
				}
			}
		});
	}

	return true;
}

void GBuffer::saveAsImages(const std::wstring& outputPath) const
{
	Ref< drawing::Image > image;

	// Positions as a checker pattern.
	image = new drawing::Image(drawing::PixelFormat::getA8R8G8B8(), m_width, m_height);
	image->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));
	for (int32_t y = 0; y < m_height; ++y)
	{
		for (int32_t x = 0; x < m_width; ++x)
		{
			const Element& e = m_data[x + y * m_width];
			if (e.polygon == model::c_InvalidIndex)
				continue;

			const int32_t bx = int32_t(e.position.x() * 32.0f) & 1;
			const int32_t by = int32_t(e.position.y() * 32.0f) & 1;
			const int32_t bz = int32_t(e.position.z() * 32.0f) & 1;

			image->setPixel(x, y, (bx ^ by ^ bz) ? Color4f(1.0f, 1.0f, 1.0f, 1.0f) : Color4f(0.2f, 0.2f, 0.2f, 1.0f));
		}
	}
	image->save(outputPath + L"_Positions.png");

	// Normals.
	image = new drawing::Image(drawing::PixelFormat::getA8R8G8B8(), m_width, m_height);
	image->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));
	for (int32_t y = 0; y < m_height; ++y)
	{
		for (int32_t x = 0; x < m_width; ++x)
		{
			const Element& e = m_data[x + y * m_width];
			if (e.polygon == model::c_InvalidIndex)
				continue;

			const Vector4 n = e.normal * Scalar(0.5f) + Scalar(0.5f);
			image->setPixel(x, y, Color4f(n.x(), n.y(), n.z(), 1.0f));
		}
	}
	image->save(outputPath + L"_Normals.png");

	// Deltas.
	image = new drawing::Image(drawing::PixelFormat::getA8R8G8B8(), m_width, m_height);
	image->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));
	for (int32_t y = 0; y < m_height; ++y)
	{
		for (int32_t x = 0; x < m_width; ++x)
		{
			const Element& e = m_data[x + y * m_width];
			if (e.polygon == model::c_InvalidIndex)
				continue;

			const float n = e.delta * 0.1f;
			image->setPixel(x, y, Color4f(n, n, n, 1.0f));
		}
	}
	image->save(outputPath + L"_Deltas.png");

	// Polygons
	image = new drawing::Image(drawing::PixelFormat::getA8R8G8B8(), m_width, m_height);
	image->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));

	SmallMap< uint32_t, Color4f > colors;
	Random rnd(std::clock());

	for (int32_t y = 0; y < m_height; ++y)
	{
		for (int32_t x = 0; x < m_width; ++x)
		{
			const Element& e = m_data[x + y * m_width];
			if (e.polygon == model::c_InvalidIndex)
				continue;

			if (colors.find(e.polygon) == colors.end())
			{
				colors[e.polygon] = Color4f(
					rnd.nextFloat(),
					rnd.nextFloat(),
					rnd.nextFloat(),
					1.0f
				);
			}

			image->setPixel(x, y, colors[e.polygon]);
		}
	}
	image->save(outputPath + L"_Polygons.png");
}

	}
}
