#include <algorithm>
#include <functional>
#include <vector>
#include "Core/Log/Log.h"
#include "Core/Math/Aabb2.h"
#include "Core/Math/Format.h"
#include "Core/Math/Triangulator.h"
#include "Core/Math/Winding2.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Illuminate/Editor/GBuffer.h"

namespace traktor
{
	namespace illuminate
	{
		namespace
		{

void triangleTop(const Vector2& v1, const Vector2& v2, const Vector2& v3, const std::function< void(float, float) >& fn)
{
	Vector2 mn = min(v1, min(v2, v3));
	Vector2 mx = max(v1, max(v2, v3));

	if (std::fabs(v2.y - v1.y) <= 1.0f)
	{
		for (float y = std::floor(mn.y); y <= std::ceil(mx.y); y += 1.0f)
		{
			for (float x = mn.x - 1.0f; x <= mx.x + 1.0f; ++x)
				fn(x, y);
		}
	}
	else
	{
		float invslope1 = (v2.x - v1.x) / (v2.y - v1.y);
		float invslope2 = (v3.x - v1.x) / (v3.y - v1.y);

		if (invslope1 < invslope2)
		{
			float tmp = invslope1;
			invslope1 = invslope2;
			invslope2 = tmp;
		}

		float curx1 = v1.x;
		float curx2 = v1.x;

		float bias1 = std::fabs(invslope1);
		float bias2 = std::fabs(invslope2);

		float scanlineY = std::floor(v1.y);
		for (; scanlineY <= std::ceil(v2.y); scanlineY += 1.0f)
		{
			float sx = std::floor(curx2 - bias2) - 1.0f;
			float ex = std::ceil(curx1 + bias1) + 1.0f;

			if (sx < mn.x - 2.0f)
				sx = mn.x - 2.0f;

			if (ex > mx.x + 2.0f)
				ex = mx.x + 2.0f;

			for (float x = sx; x <= ex; ++x)
				fn(x, scanlineY);

			curx1 += invslope1;
			curx2 += invslope2;
		}
	}
}

void triangleBottom(const Vector2& v1, const Vector2& v2, const Vector2& v3, const std::function< void(float, float) >& fn)
{
	Vector2 mn = min(v1, min(v2, v3));
	Vector2 mx = max(v1, max(v2, v3));

	if (std::fabs(v2.y - v3.y) <= 1.0f)
	{
		for (float y = std::floor(mn.y); y <= std::ceil(mx.y); y += 1.0f)
		{
			for (float x = mn.x - 1.0f; x <= mx.x + 1.0f; ++x)
				fn(x, y);
		}
	}
	else
	{
		float invslope1 = (v3.x - v1.x) / (v3.y - v1.y);
		float invslope2 = (v3.x - v2.x) / (v3.y - v2.y);

		if (invslope1 > invslope2)
		{
			float tmp = invslope1;
			invslope1 = invslope2;
			invslope2 = tmp;
		}

		float curx1 = v3.x;
		float curx2 = v3.x;

		float bias1 = std::fabs(invslope1);
		float bias2 = std::fabs(invslope2);

		float scanlineY = std::ceil(v3.y) + 1.0f;
		for (; scanlineY > std::floor(v1.y); scanlineY -= 1.0f)
		{
			float sx = std::floor(curx2 - bias2) - 1.0f;
			float ex = std::ceil(curx1 + bias1) + 1.0f;

			if (sx < mn.x - 2.0f)
				sx = mn.x - 2.0f;

			if (ex > mx.x + 2.0f)
				ex = mx.x + 2.0f;

			for (float x = sx; x <= ex; ++x)
				fn(x, scanlineY);

			curx1 -= invslope1;
			curx2 -= invslope2;
		}
	}
}

void triangleVisit(const Vector2& v1, const Vector2& v2, const Vector2& v3, const std::function< void(float, float) >& fn)
{
	std::vector< Vector2 > v(3);
	v[0] = v1;
	v[1] = v2;
	v[2] = v3;

	std::sort(v.begin(), v.end(), [](const Vector2& a, const Vector2& b) {
		return a.y < b.y;
	});

	Vector2 vm(
		v[0].x + ((v[1].y - v[0].y) / (v[2].y - v[0].y)) * (v[2].x - v[0].x),
		v[1].y
	);
	triangleTop(v[0], v[1], vm, fn);
	triangleBottom(v[1], vm, v[2], fn);
}


Vector2 closestOnLine(const Vector2& v1, const Vector2& v2, const Vector2& pt)
{
	Vector2 d = v2 - v1;
	Vector2 p = d.perpendicular().normalized();
	float k = dot(p, pt - v1);

	Vector2 pt0 = pt - p * k;

	float k2 = dot(d, pt0- v1) / (d.length() * d.length());
	if (k2 >= 0 && k2 <= 1)
		return pt0;
	else if (k2 < 0)
		return v1;
	else
		return v2;
}

Vector2 closestOnTriangle(const Vector2& v1, const Vector2& v2, const Vector2& v3, const Vector2& pt)
{
	Vector2 p1 = closestOnLine(v1, v2, pt);
	Vector2 p2 = closestOnLine(v2, v3, pt);
	Vector2 p3 = closestOnLine(v3, v1, pt);

	float ln1 = (p1 - pt).length();
	float ln2 = (p2 - pt).length();
	float ln3 = (p3 - pt).length();

	if (ln1 < ln2)
	{
		if (ln1 < ln3)
			return p1;
	}
	else
	{
		if (ln2 < ln3)
			return p2;
	}

	return p3;
}


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
		float denom = (v1.y - v2.y) * (v0.x - v2.x) + (v2.x - v1.x) * (v0.y - v2.y);
		m_invDenom = 1.0f / denom;
	}

	Vector4 factors(const Vector2& pt) const
	{
		float alpha = ((m_v1.y - m_v2.y) * (pt.x - m_v2.x) + (m_v2.x - m_v1.x) * (pt.y - m_v2.y)) * m_invDenom;
		float beta = ((m_v2.y - m_v0.y) * (pt.x - m_v2.x) + (m_v0.x - m_v2.x) * (pt.y - m_v2.y)) * m_invDenom;
		float gamma = 1.0f - alpha - beta;
		return Vector4(alpha, beta, gamma, 0.0f);
	}

	bool inside(const Vector2& pt) const
	{
		Vector4 f = factors(pt).xyz1();
		return compareAllGreaterEqual(f, Vector4::zero());
	}

private:
	Vector2 m_v0;
	Vector2 m_v1;
	Vector2 m_v2;
	float m_invDenom;
};

class Interpolants
{
public:
	Interpolants(
		const Vector4 v0,
		const Vector4 v1,
		const Vector4 v2
	)
	:	m_v0(v0)
	,	m_v1(v1)
	,	m_v2(v2)
	{
	}

	Vector4 evaluate(const Barycentric& bary, const Vector2& pt) const
	{
		Vector4 f = bary.factors(pt);
		return m_v0 * f.x() + m_v1 * f.y() + m_v2 * f.z();
	}

private:
	Vector4 m_v0;
	Vector4 m_v1;
	Vector4 m_v2;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.illuminate.GBuffer", GBuffer, Object)

GBuffer::GBuffer()
:	m_width(0)
,	m_height(0)
{
}

bool GBuffer::create(int32_t width, int32_t height, const model::Model& model, const Transform& transform, uint32_t texCoordChannel)
{
	m_width = width;
	m_height = height;
	m_data.resize(width * height);

	for (uint32_t i = 0; i < model.getPolygonCount(); ++i)
	{
		const auto& polygon = model.getPolygon(i);

		// Extract data for polygon.
		AlignedVector< Vector4 > positions;
		AlignedVector< Vector4 > normals;
		Winding2 texCoords;

		for (const auto index : polygon.getVertices())
		{
			const auto& vertex = model.getVertex(index);

			uint32_t positionIndex = vertex.getPosition();
			positions.push_back(transform * model.getPosition(positionIndex).xyz1());

			uint32_t normalIndex = vertex.getNormal();
			normals.push_back(transform * model.getNormal(normalIndex).xyz0());

			uint32_t texCoordIndex = vertex.getTexCoord(texCoordChannel);
			texCoords.points.push_back(
				model.getTexCoord(texCoordIndex) * Vector2(width, height) - Vector2(0.5f, 0.5f) // - 1, height - 1)
			);
		}

		// Triangulate winding so we can easily traverse lightmap fragments.
		AlignedVector< Triangulator::Triangle > triangles;
		Triangulator().freeze(texCoords.points, triangles);

		// Trace triangle interiors.
		for (const auto& triangle : triangles)
		{
			size_t i0 = triangle.indices[0];
			size_t i1 = triangle.indices[1];
			size_t i2 = triangle.indices[2];

			Barycentric bary(
				texCoords.points[i0],
				texCoords.points[i1],
				texCoords.points[i2]
			);

			Interpolants ipolPositions(
				positions[i0],
				positions[i1],
				positions[i2]
			);

			Interpolants ipolNormals(
				normals[i0],
				normals[i1],
				normals[i2]
			);

			triangleVisit(
				texCoords.points[i0],
				texCoords.points[i1],
				texCoords.points[i2],
				[&](float x, float y)
				{

					int32_t ix = (int32_t)x;
					int32_t iy = (int32_t)y;
					if (ix >= 0 && iy >= 0 && ix < width && iy < height)
					{
						Vector2 cpt(ix, iy);

						bool inside = bary.inside(cpt);

						// if (!inside)
						// {
						// 	cpt = closestOnTriangle(
						// 		texCoords.points[i0],
						// 		texCoords.points[i1],
						// 		texCoords.points[i2],
						// 		cpt
						// 	);	
						// }

						auto& elm = m_data[x + y * m_width];

						if (!inside && elm.polygon != model::c_InvalidIndex)
							return;

						elm.polygon = i;
						elm.material = polygon.getMaterial();
						elm.position = ipolPositions.evaluate(bary, cpt).xyz1();
						elm.normal = ipolNormals.evaluate(bary, cpt).xyz0().normalized();
						elm.delta = ipolPositions.evaluate(bary, cpt + Vector2(1.0f, 1.0f)).xyz1() - elm.position;
						elm.inside = inside;

					}

				}
			);

/*
			for (int32_t y = y0; y <= y1; ++y)
			{
				for (int32_t x = x0; x <= x1; ++x)
				{
					const Vector2 pt(x, y);
					
					bool inside = false;
					for (int32_t dy = -1; dy <= 1; ++dy)
					{
						for (int32_t dx = -1; dx <= 1; ++dx)
							inside |= bary.inside(pt + Vector2(dx, dy));
					}

					if (inside)
					{
						auto& elm = m_data[x + y * m_width];
						bool center = bary.inside(pt);
						if (center || elm.polygon == model::c_InvalidIndex)
						{
							Vector2 cpt = center ? pt : texCoords.closest(pt);

							elm.polygon = i;
							elm.material = polygon.getMaterial();
							elm.position = ipolPositions.evaluate(bary, cpt).xyz1();
							elm.normal = ipolNormals.evaluate(bary, cpt).xyz0().normalized();
							elm.delta = ipolPositions.evaluate(bary, cpt + Vector2(1.0f, 1.0f)).xyz1() - elm.position;
						}
					}
				}
			}
*/
		}
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

			int32_t bx = int32_t(e.position.x() * 32.0f) & 1;
			int32_t by = int32_t(e.position.y() * 32.0f) & 1;
			int32_t bz = int32_t(e.position.z() * 32.0f) & 1;

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

			Vector4 n = e.normal * Scalar(0.5f) + Scalar(0.5f);
			image->setPixel(x, y, Color4f(n.x(), n.y(), n.z(), 1.0f));
		}
	}
	image->save(outputPath + L"_Normals.png");
}

	}
}
