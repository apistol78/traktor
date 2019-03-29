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
			texCoords.points.push_back(model.getTexCoord(texCoordIndex) * Vector2(width - 1, height - 1));
		}

		// Triangulate winding so we can easily traverse lightmap fragments.
		AlignedVector< Triangulator::Triangle > triangles;
		Triangulator().freeze(texCoords.points, triangles);

		// Trace edges first, store count in alpha so we can average.
		for (const auto& triangle : triangles)
		{
			size_t i0 = triangle.indices[0];
			size_t i1 = triangle.indices[1];
			size_t i2 = triangle.indices[2];

			const Vector4& p0 = positions[i0];
			const Vector4& p1 = positions[i1];
			const Vector4& p2 = positions[i2];

			const Vector4& n0 = normals[i0];
			const Vector4& n1 = normals[i1];
			const Vector4& n2 = normals[i2];

			const Vector2& tc0 = texCoords.points[i0];
			const Vector2& tc1 = texCoords.points[i1];
			const Vector2& tc2 = texCoords.points[i2];

			Aabb2 aabb;
			aabb.contain(tc0);
			aabb.contain(tc1);
			aabb.contain(tc2);

			int32_t x0 = int32_t(aabb.mn.x);
			int32_t x1 = int32_t(aabb.mx.x + 1);
			int32_t y0 = int32_t(aabb.mn.y);
			int32_t y1 = int32_t(aabb.mx.y + 1);

			float denom = (tc1.y - tc2.y) * (tc0.x - tc2.x) + (tc2.x - tc1.x) * (tc0.y - tc2.y);
			float invDenom = 1.0f / denom;

			Winding2 wt;
			wt.points.resize(3);
			wt.points[0] = tc0;
			wt.points[1] = tc1;
			wt.points[2] = tc2;

			for (int32_t y = y0; y <= y1; ++y)
			{
				for (int32_t x = x0; x <= x1; ++x)
				{
					const Vector2 pt(x + 0.5f, y + 0.5f);

					bool inside1 = 
						wt.inside(Vector2(x, y)) ||
						wt.inside(Vector2(x + 1, y)) ||
						wt.inside(Vector2(x, y + 1)) ||
						wt.inside(Vector2(x + 1, y + 1));

					float alpha = ((tc1.y - tc2.y) * (pt.x - tc2.x) + (tc2.x - tc1.x) * (pt.y - tc2.y)) * invDenom;
					float beta = ((tc2.y - tc0.y) * (pt.x - tc2.x) + (tc0.x - tc2.x) * (pt.y - tc2.y)) * invDenom;
					float gamma = 1.0f - alpha - beta;

					bool inside2 = (alpha >= 0.0f && beta >= 0.0f && gamma >= 0.0f);

					if (inside1 && !inside2)
					{
						Vector4 position = (p0 * Scalar(alpha) + p1 * Scalar(beta) + p2 * Scalar(gamma)).xyz1();
						Vector4 normal = (n0 * Scalar(alpha) + n1 * Scalar(beta) + n2 * Scalar(gamma)).xyz0().normalized();

						auto& elm = m_data[x + y * m_width];
						elm.polygon = i;
						elm.material = polygon.getMaterial();
						elm.position = position;
						elm.normal = normal;
					}
				}
			}
		}
		
		// Trace triangle interiors.
		for (const auto& triangle : triangles)
		{
			size_t i0 = triangle.indices[0];
			size_t i1 = triangle.indices[1];
			size_t i2 = triangle.indices[2];

			const Vector4& p0 = positions[i0];
			const Vector4& p1 = positions[i1];
			const Vector4& p2 = positions[i2];

			const Vector4& n0 = normals[i0];
			const Vector4& n1 = normals[i1];
			const Vector4& n2 = normals[i2];

			const Vector2& tc0 = texCoords.points[i0];
			const Vector2& tc1 = texCoords.points[i1];
			const Vector2& tc2 = texCoords.points[i2];

			Aabb2 aabb;
			aabb.contain(tc0);
			aabb.contain(tc1);
			aabb.contain(tc2);

			int32_t x0 = int32_t(aabb.mn.x);
			int32_t x1 = int32_t(aabb.mx.x + 1);
			int32_t y0 = int32_t(aabb.mn.y);
			int32_t y1 = int32_t(aabb.mx.y + 1);

			float denom = (tc1.y - tc2.y) * (tc0.x - tc2.x) + (tc2.x - tc1.x) * (tc0.y - tc2.y);
			float invDenom = 1.0f / denom;

			for (int32_t y = y0; y <= y1; ++y)
			{
				for (int32_t x = x0; x <= x1; ++x)
				{
					const Vector2 pt(x + 0.5f, y + 0.5f);

					float alpha = ((tc1.y - tc2.y) * (pt.x - tc2.x) + (tc2.x - tc1.x) * (pt.y - tc2.y)) * invDenom;
					float beta = ((tc2.y - tc0.y) * (pt.x - tc2.x) + (tc0.x - tc2.x) * (pt.y - tc2.y)) * invDenom;
					float gamma = 1.0f - alpha - beta;

					bool inside = (alpha >= 0.0f && beta >= 0.0f && gamma >= 0.0f);
					if (inside)
					{
						Vector4 position = (p0 * Scalar(alpha) + p1 * Scalar(beta) + p2 * Scalar(gamma)).xyz1();
						Vector4 normal = (n0 * Scalar(alpha) + n1 * Scalar(beta) + n2 * Scalar(gamma)).xyz0().normalized();

						auto& elm = m_data[x + y * m_width];
						elm.polygon = i;
						elm.material = polygon.getMaterial();
						elm.position = position;
						elm.normal = normal;
					}
				}
			}
		}
	}

	const int32_t c_offsets[][2] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
	for (int32_t i = 0; i < 16; ++i)
	{
		for (int32_t y = 0; y < m_height; ++y)
		{
			for (int32_t x = 0; x < m_width; ++x)
			{
				auto& elm = m_data[x + y * m_width];
				if (elm.polygon != model::c_InvalidIndex)
					continue;

				for (int32_t i = 0; i < 4; ++i)
				{
					int32_t ox = x + c_offsets[i][0];
					int32_t oy = y + c_offsets[i][1];
					if (ox >= 0 && oy >= 0 && ox < m_width && oy < m_height)
					{
						const auto& elm2 = m_data[ox + oy * m_width];
						if (elm2.polygon != model::c_InvalidIndex)
						{
							elm = elm2;
							break;
						}
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
