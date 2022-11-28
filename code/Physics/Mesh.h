/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Math/Vector4.h"
#include "Core/Containers/AlignedVector.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

}

namespace traktor::physics
{

/*! Mesh geometry.
 * \ingroup Physics
 */
class T_DLLCLASS Mesh : public Object
{
	T_RTTI_CLASS;

public:
#pragma pack(1)
	struct Triangle
	{
		uint32_t indices[3];
		uint32_t material;
	};
#pragma pack()

#pragma pack(1)
	struct Material
	{
		float friction;
		float restitution;
	};
#pragma pack()

	Mesh();

	void setVertices(const AlignedVector< Vector4 >& vertices);

	const AlignedVector< Vector4 >& getVertices() const;

	void setNormals(const AlignedVector< Vector4 >& normals);

	const AlignedVector< Vector4 >& getNormals() const;

	void setShapeTriangles(const AlignedVector< Triangle >& shapeTriangles);

	const AlignedVector< Triangle >& getShapeTriangles() const;

	void setHullTriangles(const AlignedVector< Triangle >& hullTriangles);

	const AlignedVector< Triangle >& getHullTriangles() const;

	void setHullIndices(const AlignedVector< uint32_t >& hullIndices);

	const AlignedVector< uint32_t >& getHullIndices() const;

	void setMaterials(const AlignedVector< Material >& materials);

	const AlignedVector< Material >& getMaterials() const;

	void setOffset(const Vector4& offset);

	const Vector4& getOffset() const;

	void setMargin(float margin);

	float getMargin() const;

	bool read(IStream* stream);

	bool write(IStream* stream);

private:
	AlignedVector< Vector4 > m_vertices;
	AlignedVector< Vector4 > m_normals;
	AlignedVector< Triangle > m_shapeTriangles;
	AlignedVector< Triangle > m_hullTriangles;
	AlignedVector< uint32_t > m_hullIndices;
	AlignedVector< Material > m_materials;
	Vector4 m_offset;
	float m_margin;
};

}
