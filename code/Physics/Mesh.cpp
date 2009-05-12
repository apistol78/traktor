#include "Physics/Mesh.h"
#include "Core/Io/Stream.h"
#include "Core/Io/Reader.h"
#include "Core/Io/Writer.h"

namespace traktor
{
	namespace physics
	{
		namespace
		{

const uint32_t c_version = 2;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.Mesh", Mesh, Object)

void Mesh::setVertices(const AlignedVector< Vector4 >& vertices)
{
	m_vertices = vertices;
}

const AlignedVector< Vector4 >& Mesh::getVertices() const
{
	return m_vertices;
}

void Mesh::setShapeTriangles(const std::vector< Triangle >& shapeTriangles)
{
	m_shapeTriangles = shapeTriangles;
}

const std::vector< Mesh::Triangle >& Mesh::getShapeTriangles() const
{
	return m_shapeTriangles;
}

void Mesh::setHullTriangles(const std::vector< Triangle >& hullTriangles)
{
	m_hullTriangles = hullTriangles;
}

const std::vector< Mesh::Triangle >& Mesh::getHullTriangles() const
{
	return m_hullTriangles;
}

bool Mesh::read(Stream* stream)
{
	Reader rd(stream);

	uint32_t version;
	rd >> version;

	if (version != c_version)
		return false;

	uint32_t vertexCount;
	rd >> vertexCount;

	uint32_t shapeTriangleCount;
	rd >> shapeTriangleCount;

	uint32_t hullTriangleCount;
	rd >> hullTriangleCount;

	m_vertices.resize(vertexCount);
	rd.read(&m_vertices[0], vertexCount, sizeof(Vector4));

	m_shapeTriangles.resize(shapeTriangleCount);
	rd.read(&m_shapeTriangles[0], shapeTriangleCount, sizeof(Triangle));

	m_hullTriangles.resize(hullTriangleCount);
	rd.read(&m_hullTriangles[0], hullTriangleCount, sizeof(Triangle));

	return true;
}

bool Mesh::write(Stream* stream)
{
	Writer wr(stream);

	wr << uint32_t(c_version);
	wr << uint32_t(m_vertices.size());
	wr << uint32_t(m_shapeTriangles.size());
	wr << uint32_t(m_hullTriangles.size());
	
	if (!m_vertices.empty())
		wr.write(&m_vertices[0], int(m_vertices.size()), sizeof(Vector4));

	if (!m_shapeTriangles.empty())
		wr.write(&m_shapeTriangles[0], int(m_shapeTriangles.size()), sizeof(Triangle));

	if (!m_hullTriangles.empty())
		wr.write(&m_hullTriangles[0], int(m_hullTriangles.size()), sizeof(Triangle));

	return true;
}

	}
}
