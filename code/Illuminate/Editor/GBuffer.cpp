#include "Core/Math/Aabb2.h"
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
				model.getTexCoord(texCoordIndex) * Vector2(width - 1, height - 1)
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

			Aabb2 aabb;
			aabb.contain(texCoords.points[i0]);
			aabb.contain(texCoords.points[i1]);
			aabb.contain(texCoords.points[i2]);

			int32_t x0 = int32_t(aabb.mn.x);
			int32_t x1 = int32_t(aabb.mx.x + 1);
			int32_t y0 = int32_t(aabb.mn.y);
			int32_t y1 = int32_t(aabb.mx.y + 1);

			for (int32_t y = y0; y <= y1; ++y)
			{
				for (int32_t x = x0; x <= x1; ++x)
				{
					const Vector2 pt(x, y);
					
					bool inside = false;
					for (int32_t dy = -3; dy <= 3; ++dy)
					{
						for (int32_t dx = -3; dx <= 3; ++dx)
							inside |= bary.inside(pt + Vector2(dx, dy));
					}

					if (inside)
					{
						Vector2 cpt = pt; //texCoords.closest(pt);

						auto& elm = m_data[x + y * m_width];
						elm.polygon = i;
						elm.material = polygon.getMaterial();
						elm.position = ipolPositions.evaluate(bary, cpt).xyz1();
						elm.normal = ipolNormals.evaluate(bary, cpt).xyz0().normalized();
						elm.delta = ipolPositions.evaluate(bary, cpt + Vector2(1.0f, 1.0f)).xyz1() - elm.position;
					}
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
}

	}
}
