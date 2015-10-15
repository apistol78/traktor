#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/RenderMeshFactory.h"
#include "Resource/IResourceManager.h"
#include "Spark/Canvas.h"
#include "Spark/Context.h"
#include "Spark/Shape.h"
#include "Spark/Triangulator.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

const float c_controlPoints[3][2] =
{
	{ 0.0f, 0.0f },
	{ 0.5f, 0.0f },
	{ 1.0f, 1.0f }
};

#pragma pack(1)
struct Vertex
{
	float position[2];
	float texCoord[2];
	float controlPoints[2];
};
#pragma pack()

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Canvas", Canvas, Object)

void Canvas::clear()
{
	m_path = Path();
}

void Canvas::moveTo(float x, float y)
{
	m_path.moveTo(x, y);
}

void Canvas::lineTo(float x, float y)
{
	m_path.lineTo(x, y);
}

void Canvas::quadricTo(float x1, float y1, float x, float y)
{
	m_path.quadricTo(x1, y1, x, y);
}

void Canvas::quadricTo(float x, float y)
{
	m_path.quadricTo(x, y);
}

void Canvas::cubicTo(float x1, float y1, float x2, float y2, float x, float y)
{
	m_path.cubicTo(x1, y1, x2, y2, x, y);
}

void Canvas::cubicTo(float x2, float y2, float x, float y)
{
	m_path.cubicTo(x2, y2, x, y);
}

void Canvas::close()
{
	m_path.close();
}

void Canvas::rect(float x, float y, float width, float height, float radius)
{
	if (radius <= 0.0f)
	{
		m_path.moveTo(x, y);
		m_path.lineTo(x + width, y);
		m_path.lineTo(x + width, y + height);
		m_path.lineTo(x, y + height);
		m_path.close();
	}
	else
	{
		m_path.moveTo(x + radius, y);
		m_path.lineTo(x + width - radius, y);
		m_path.cubicTo(x + width, y, x + width, y, x + width, y + radius);
		m_path.lineTo(x + width, y + height - radius);
		m_path.cubicTo(x + width, y + height, x + width, y + height, x + width - radius, y + height);
		m_path.lineTo(x + radius, y + height);
		m_path.cubicTo(x, y + height, x, y + height, x, y + height - radius);
		m_path.lineTo(x, y + radius);
		m_path.cubicTo(x, y, x, y, x + radius, y);
		m_path.close();
	}
}

void Canvas::circle(float x, float y, float radius)
{
	const float c_circleMagic = 0.5522847498f;
	float rk = radius * c_circleMagic;

	m_path.moveTo(x, y + radius);
	m_path.cubicTo(
		x + rk, y + radius,
		x + radius, y + rk,
		x + radius, y
	);
	m_path.cubicTo(
		x + radius, y - rk,
		x + rk, y - radius,
		x, y - radius
	);
	m_path.cubicTo(
		x - rk, y - radius,
		x - radius, y - rk,
		x - radius, y
	);
	m_path.cubicTo(
		x - radius, y + rk,
		x - rk, y + radius,
		x, y + radius
	);
	m_path.close();
}

void Canvas::fill(const Color4f& fillColor)
{
	Batch b;
	b.fillColor = fillColor;
	Triangulator().triangulate(&m_path, b.triangles);
	if (!b.triangles.empty())
		m_batches.push_back(b);
}

void Canvas::stroke()
{
}

Ref< Shape > Canvas::createShape(const Context* context) const
{
	// Bind default shape shader.
	resource::Proxy< render::Shader > shader;
	if (!context->getResourceManager()->bind(
		resource::Id< render::Shader >(Guid(L"{E411A034-2FDA-4B44-A378-700D1CB8B6E4}")),
		shader
	))
		return 0;

	// Count number of triangles in canvas.
	uint32_t triangleCount = 0;
	for (std::list< Batch >::const_iterator i = m_batches.begin(); i != m_batches.end(); ++i)
		triangleCount += uint32_t(i->triangles.size());
	if (!triangleCount)
		return 0;

	// Define vertex layout.
	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat2, offsetof(Vertex, position)));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat2, offsetof(Vertex, texCoord), 0));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat2, offsetof(Vertex, controlPoints), 1));

	// Create render mesh.
	Ref< render::Mesh > mesh = render::RenderMeshFactory(context->getRenderSystem()).createMesh(
		vertexElements,
		triangleCount * 3 * render::getVertexSize(vertexElements),
		render::ItUInt16,
		0
	);

	std::vector< render::Mesh::Part > meshParts;
	AlignedVector< Shape::Part > shapeParts;
	uint32_t vertexOffset = 0;
	uint32_t vertexCount = 0;

	// Fill vertex buffer.
	Vertex* vertex = static_cast< Vertex* >(mesh->getVertexBuffer()->lock());
	for (std::list< Batch >::const_iterator i = m_batches.begin(); i != m_batches.end(); ++i)
	{
		// Fill
		vertexCount = 0;
		for (AlignedVector< Triangulator::Triangle >::const_iterator j = i->triangles.begin(); j != i->triangles.end(); ++j)
		{
			if (j->type == Triangulator::TcFill)
			{
				for (uint32_t k = 0; k < 3; ++k)
				{
					Vector2 v = j->v[k].toVector2();
					vertex->position[0] = v.x;
					vertex->position[1] = v.y;
					vertex++;
					vertexCount++;
				}
			}
		}
		if (vertexCount > 0)
		{
			render::Mesh::Part meshPart;
			meshPart.primitives.setNonIndexed(render::PtTriangles, vertexOffset, uint32_t(vertexCount / 3));
			meshParts.push_back(meshPart);

			Shape::Part shapePart;
			shapePart.fillColor = i->fillColor;
			shapePart.curveSign = 0;
			shapeParts.push_back(shapePart);

			vertexOffset += vertexCount;
		}

		// In
		vertexCount = 0;
		for (AlignedVector< Triangulator::Triangle >::const_iterator j = i->triangles.begin(); j != i->triangles.end(); ++j)
		{
			if (j->type == Triangulator::TcIn)
			{
				for (uint32_t k = 0; k < 3; ++k)
				{
					Vector2 v = j->v[k].toVector2();
					vertex->position[0] = v.x;
					vertex->position[1] = v.y;
					vertex->controlPoints[0] = c_controlPoints[k][0];
					vertex->controlPoints[1] = c_controlPoints[k][1];
					vertex++;
					vertexCount++;
				}
			}
		}
		if (vertexCount > 0)
		{
			render::Mesh::Part meshPart;
			meshPart.primitives.setNonIndexed(render::PtTriangles, vertexOffset, uint32_t(vertexCount / 3));
			meshParts.push_back(meshPart);

			Shape::Part shapePart;
			shapePart.fillColor = i->fillColor;
			shapePart.curveSign = 1;
			shapeParts.push_back(shapePart);

			vertexOffset += vertexCount;
		}

		// Out
		vertexCount = 0;
		for (AlignedVector< Triangulator::Triangle >::const_iterator j = i->triangles.begin(); j != i->triangles.end(); ++j)
		{
			if (j->type == Triangulator::TcOut)
			{
				for (uint32_t k = 0; k < 3; ++k)
				{
					Vector2 v = j->v[k].toVector2();
					vertex->position[0] = v.x;
					vertex->position[1] = v.y;
					vertex->controlPoints[0] = c_controlPoints[k][0];
					vertex->controlPoints[1] = c_controlPoints[k][1];
					vertex++;
					vertexCount++;
				}
			}
		}
		if (vertexCount > 0)
		{
			render::Mesh::Part meshPart;
			meshPart.primitives.setNonIndexed(render::PtTriangles, vertexOffset, uint32_t(vertexCount / 3));
			meshParts.push_back(meshPart);

			Shape::Part shapePart;
			shapePart.fillColor = i->fillColor;
			shapePart.curveSign = -1;
			shapeParts.push_back(shapePart);

			vertexOffset += vertexCount;
		}
	}
	mesh->getVertexBuffer()->unlock();

	mesh->setParts(meshParts);
	mesh->setBoundingBox(Aabb3());

	// Create shape.
	Ref< Shape > shape = new Shape(
		mesh,
		shader,
		shapeParts,
		Aabb2()
	);

	return shape;
}

	}
}
