/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/StaticVector.h"
#include "Core/Serialization/ISerializable.h"
#include "Model/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MODEL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializer;

}

namespace traktor::model
{

/*! Polygon
 * \ingroup Model
 */
class T_DLLCLASS Polygon : public ISerializable
{
	T_RTTI_CLASS;

public:
	typedef StaticVector< uint32_t, 32 > vertices_t;

	Polygon() = default;

	Polygon(uint32_t material, uint32_t vertex1, uint32_t vertex2);

	Polygon(uint32_t material, uint32_t vertex1, uint32_t vertex2, uint32_t vertex3);

	Polygon(uint32_t material, uint32_t vertex1, uint32_t vertex2, uint32_t vertex3, uint32_t vertex4);

	void setMaterial(uint32_t material);

	uint32_t getMaterial() const { return m_material; }

	void setNormal(uint32_t normal);

	uint32_t getNormal() const { return m_normal; }

	void setSmoothGroup(uint32_t smoothGroup);

	uint32_t getSmoothGroup() const { return m_smoothGroup; }

	void clearVertices();

	void flipWinding();

	void addVertex(uint32_t vertex);

	/*! Insert vertex at index. */
	void insertVertex(uint32_t index, uint32_t vertex);

	void setVertex(uint32_t index, uint32_t vertex);

	uint32_t getVertex(uint32_t index) const { return m_vertices[index]; }

	uint32_t getVertexCount() const { return (uint32_t)m_vertices.size(); }

	void setVertices(const vertices_t& vertices);

	const vertices_t& getVertices() const { return m_vertices; }

	vertices_t& getVertices() { return m_vertices; }

	virtual void serialize(ISerializer& s) override final;

	bool operator == (const Polygon& r) const;

private:
	uint32_t m_material = c_InvalidIndex;
	uint32_t m_normal = c_InvalidIndex;
	uint32_t m_smoothGroup = 0;
	vertices_t m_vertices;
};

}
