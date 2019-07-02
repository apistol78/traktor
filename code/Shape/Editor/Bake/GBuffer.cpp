#include <algorithm>
#include <ctime>
#include <functional>
#include <vector>
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
/*
void triangleTop(const Vector2& v1, const Vector2& v2, const Vector2& v3, const std::function< void(float, float) >& fn)
{
	Vector2 mn = min(v1, min(v2, v3));
	Vector2 mx = max(v1, max(v2, v3));

	if (std::fabs(v2.y - v1.y) <= 1.0f)
	{
		for (float y = std::floor(mn.y - 0.5f); y <= std::ceil(mx.y + 0.5f); y += 1.0f)
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
		for (float y = std::floor(mn.y - 0.5f); y <= std::ceil(mx.y + 0.5f); y += 1.0f)
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
}*/

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

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.GBuffer", GBuffer, Object)

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
	m_boundingBox = Aabb3();

	for (uint32_t i = 0; i < width * height; ++i)
	{
		m_data[i].polygon = model::c_InvalidIndex;
		m_data[i].material = 0;
		m_data[i].position = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		m_data[i].normal = Vector4(0.0f, 1.0f, 0.0f, 0.0f);
		m_data[i].delta = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	}

	for (uint32_t i = 0; i < model.getPolygonCount(); ++i)
	{
		const auto& polygon = model.getPolygon(i);

		// Extract data for polygon.
		AlignedVector< Vector4 > positions;
		AlignedVector< Vector4 > normals;
		Winding2 texCoords;
		Aabb2 texBounds;

		for (const auto index : polygon.getVertices())
		{
			const auto& vertex = model.getVertex(index);

			uint32_t positionIndex = vertex.getPosition();
			positions.push_back(transform * model.getPosition(positionIndex).xyz1());

			uint32_t normalIndex = vertex.getNormal();
			normals.push_back(transform * model.getNormal(normalIndex).xyz0());

			uint32_t texCoordIndex = vertex.getTexCoord(texCoordChannel);
			texCoords.points.push_back(
				model.getTexCoord(texCoordIndex) * Vector2(width, height) - Vector2(0.5f, 0.5f)
			);
			texBounds.contain(texCoords.points.back());

			m_boundingBox.contain(positions.back());
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

			for (int32_t x = 0; x < width; ++x)
			{
				for (int32_t y = 0; y < height; ++y)
				{
					Vector2 cpt(x, y);

					bool inside = bary.inside(cpt);
					for (int32_t iy = -2; !inside && iy <= 2; ++iy)
					{
						for (int32_t ix = -2; !inside && ix <= 2; ++ix)
						{
							inside |= bary.inside(cpt + Vector2(ix / 2.0f, iy / 2.0f));
						}
					}
					if (!inside)
						continue;

					Element& elm = m_data[x + y * m_width];

					if (elm.polygon != model::c_InvalidIndex)
					{
						if (!bary.inside(cpt))
							continue;
					}

					elm.polygon = i;
					elm.material = polygon.getMaterial();
					elm.position = ipolPositions.evaluate(bary, cpt).xyz1();
					elm.normal = ipolNormals.evaluate(bary, cpt).xyz0().normalized();
					elm.delta = ipolPositions.evaluate(bary, cpt + Vector2(1.0f, 1.0f)).xyz1() - elm.position;
				}
			}
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
