/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/Mesh.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Reader.h"
#include "Core/Io/Writer.h"

namespace traktor
{
	namespace physics
	{
		namespace
		{

const uint32_t c_version = 7;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.Mesh", Mesh, Object)

Mesh::Mesh()
:	m_margin(0.0f)
{
}

void Mesh::setVertices(const AlignedVector< Vector4 >& vertices)
{
	m_vertices = vertices;
}

const AlignedVector< Vector4 >& Mesh::getVertices() const
{
	return m_vertices;
}

void Mesh::setNormals(const AlignedVector< Vector4 >& normals)
{
	m_normals = normals;
}

const AlignedVector< Vector4 >& Mesh::getNormals() const
{
	return m_normals;
}

void Mesh::setShapeTriangles(const AlignedVector< Triangle >& shapeTriangles)
{
	m_shapeTriangles = shapeTriangles;
}

const AlignedVector< Mesh::Triangle >& Mesh::getShapeTriangles() const
{
	return m_shapeTriangles;
}

void Mesh::setHullTriangles(const AlignedVector< Triangle >& hullTriangles)
{
	m_hullTriangles = hullTriangles;
}

const AlignedVector< Mesh::Triangle >& Mesh::getHullTriangles() const
{
	return m_hullTriangles;
}

void Mesh::setHullIndices(const AlignedVector< uint32_t >& hullIndices)
{
	m_hullIndices = hullIndices;
}

const AlignedVector< uint32_t >& Mesh::getHullIndices() const
{
	return m_hullIndices;
}

void Mesh::setMaterials(const AlignedVector< Material >& materials)
{
	m_materials = materials;
}

const AlignedVector< Mesh::Material >& Mesh::getMaterials() const
{
	return m_materials;
}

void Mesh::setOffset(const Vector4& offset)
{
	m_offset = offset;
}

const Vector4& Mesh::getOffset() const
{
	return m_offset;
}

void Mesh::setMargin(float margin)
{
	m_margin = margin;
}

float Mesh::getMargin() const
{
	return m_margin;
}

bool Mesh::read(IStream* stream)
{
	Reader rd(stream);

	uint32_t version;
	rd >> version;

	if (version != c_version)
		return false;

	uint32_t vertexCount;
	rd >> vertexCount;

	uint32_t normalCount;
	rd >> normalCount;

	uint32_t shapeTriangleCount;
	rd >> shapeTriangleCount;

	uint32_t hullTriangleCount;
	rd >> hullTriangleCount;

	uint32_t hullIndexCount;
	rd >> hullIndexCount;

	uint32_t materialCount;
	rd >> materialCount;

	m_vertices.resize(vertexCount);
	if (vertexCount > 0)
		rd.read(m_vertices.ptr(), vertexCount * 4, sizeof(float));

	m_normals.resize(normalCount);
	if (normalCount > 0)
		rd.read(m_normals.ptr(), normalCount * 4, sizeof(float));

	m_shapeTriangles.resize(shapeTriangleCount);
	if (shapeTriangleCount > 0)
		rd.read(m_shapeTriangles.ptr(), shapeTriangleCount * 4, sizeof(uint32_t));

	m_hullTriangles.resize(hullTriangleCount);
	if (hullTriangleCount > 0)
		rd.read(m_hullTriangles.ptr(), hullTriangleCount * 4, sizeof(uint32_t));

	m_hullIndices.resize(hullIndexCount);
	if (hullIndexCount > 0)
		rd.read(m_hullIndices.ptr(), hullIndexCount, sizeof(uint32_t));

	m_materials.resize(materialCount);
	if (materialCount > 0)
		rd.read(m_materials.ptr(), materialCount * 2, sizeof(float));

	float offset[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	rd >> offset[0];
	rd >> offset[1];
	rd >> offset[2];
	m_offset = Vector4::loadUnaligned(offset);

	rd >> m_margin;

	return true;
}

bool Mesh::write(IStream* stream)
{
	Writer wr(stream);

	wr << (uint32_t)c_version;
	wr << (uint32_t)m_vertices.size();
	wr << (uint32_t)m_normals.size();
	wr << (uint32_t)m_shapeTriangles.size();
	wr << (uint32_t)m_hullTriangles.size();
	wr << (uint32_t)m_hullIndices.size();
	wr << (uint32_t)m_materials.size();

	if (!m_vertices.empty())
		wr.write(&m_vertices[0], (int64_t)(m_vertices.size() * 4), sizeof(float));

	if (!m_normals.empty())
		wr.write(&m_normals[0], (int64_t)(m_normals.size() * 4), sizeof(float));

	if (!m_shapeTriangles.empty())
		wr.write(&m_shapeTriangles[0], (int64_t)(m_shapeTriangles.size() * 4), sizeof(uint32_t));

	if (!m_hullTriangles.empty())
		wr.write(&m_hullTriangles[0], (int64_t)(m_hullTriangles.size() * 4), sizeof(uint32_t));

	if (!m_hullIndices.empty())
		wr.write(&m_hullIndices[0], (int64_t)m_hullIndices.size(), sizeof(uint32_t));

	if (!m_materials.empty())
		wr.write(m_materials.c_ptr(), (int64_t)(m_materials.size() * 2), sizeof(float));

	float offset[4];
	m_offset.storeUnaligned(offset);
	wr << offset[0];
	wr << offset[1];
	wr << offset[2];

	wr << m_margin;

	return true;
}

	}
}
