/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/StaticVector.h"
#include "Core/Math/Winding3.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! BSP vertex.
 * \ingroup Core
 */
struct BspVertex
{
	Vector4 position;
	Vector4 attributes[4];
};

/*! BSP polygon.
 * \ingroup Core
 */
class T_DLLCLASS BspPolygon
{
public:
	enum Classification
	{
		Front = 1,
		Back = 2,
		Span = 3,
		Coplanar = 4
	};

	typedef StaticVector< BspVertex, 32 > vertices_t;

	BspPolygon() = default;

	explicit BspPolygon(intptr_t index, const vertices_t& vertices);

	void setIndex(intptr_t index);

	void addVertex(const BspVertex& vertex);

	void addVertex(
		const Vector4& position,
		const Vector4& attr0 = Vector4::zero(),
		const Vector4& attr1 = Vector4::zero(),
		const Vector4& attr2 = Vector4::zero(),
		const Vector4& attr3 = Vector4::zero()
	);

	void setPlane(const Plane& plane);

	bool calculatePlane();

	void flip();

	int32_t classify(const Plane& plane) const;

	void split(const Plane& plane, AlignedVector< BspPolygon >& outCoplanarFront, AlignedVector< BspPolygon >& outCoplanarBack, AlignedVector< BspPolygon >& outFront, AlignedVector< BspPolygon >& outBack) const;

	intptr_t getIndex() const { return m_index; }

	const vertices_t& getVertices() const { return m_vertices; }

	const Plane& getPlane() const { return m_plane; }

private:
	intptr_t m_index = 0;
	vertices_t m_vertices;
	Plane m_plane;
};

/*! Binary space partitioning tree.
 * \ingroup Core
 */
class T_DLLCLASS BspNode
{
public:
	BspNode() = default;

	BspNode(const BspNode& node);

	BspNode(BspNode&& node);

	virtual ~BspNode();

	void invert();

	AlignedVector< BspPolygon > clip(const AlignedVector< BspPolygon >& polygons) const;

	void clip(const BspNode& other);

	void build(const AlignedVector< BspPolygon >& polygons, bool fast);

	AlignedVector< BspPolygon > allPolygons() const;

	BspNode unioon(const BspNode& other, bool fast) const;

	BspNode intersection(const BspNode& other, bool fast) const;

	BspNode difference(const BspNode& other, bool fast) const;

	BspNode& operator = (const BspNode& node);

	BspNode& operator = (BspNode&& node);

private:
	Plane m_plane;
	AlignedVector< BspPolygon > m_polygons;
	BspNode* m_front = nullptr;
	BspNode* m_back = nullptr;

	BspNode* clone() const;
};

}
