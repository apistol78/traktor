/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
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

/*! Vertex
 * \ingroup Model
 */
class T_DLLCLASS Vertex : public ISerializable
{
	T_RTTI_CLASS;

public:
	Vertex() = default;

	explicit Vertex(uint32_t position);

	explicit Vertex(uint32_t position, uint32_t normal);

	explicit Vertex(uint32_t position, uint32_t normal, uint32_t texCoord);

	void setPosition(uint32_t position) { m_position = position; }

	uint32_t getPosition() const { return m_position; }

	void setColor(uint32_t color) { m_color = color; }

	uint32_t getColor() const { return m_color; }

	void setNormal(uint32_t normal) { m_normal = normal; }

	uint32_t getNormal() const { return m_normal; }

	void setTangent(uint32_t tangent) { m_tangent = tangent; }

	uint32_t getTangent() const { return m_tangent; }

	void setBinormal(uint32_t binormal) { m_binormal = binormal; }

	uint32_t getBinormal() const { return m_binormal; }

	void clearTexCoords();

	void setTexCoord(uint32_t channel, uint32_t texCoord);

	uint32_t getTexCoord(uint32_t channel) const;

	uint32_t getTexCoordCount() const;

	void clearJointInfluences();

	void setJointInfluence(uint32_t jointIndex, float influence);

	float getJointInfluence(uint32_t jointIndex) const;

	uint32_t getJointInfluenceCount() const;

	uint32_t getHash() const;

	virtual void serialize(ISerializer& s) override final;

	bool operator == (const Vertex& r) const;

private:
	uint32_t m_position = c_InvalidIndex;
	uint32_t m_color = c_InvalidIndex;
	uint32_t m_normal = c_InvalidIndex;
	uint32_t m_tangent = c_InvalidIndex;
	uint32_t m_binormal = c_InvalidIndex;
	StaticVector< uint32_t, 4 > m_texCoords;
	AlignedVector< float > m_jointInfluences;
};

struct VertexHashFunction
{
	static uint32_t get(const Vertex& v)
	{
		return v.getPosition();
	}
};

}
