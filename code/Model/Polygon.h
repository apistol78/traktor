#pragma once

#include "Core/Containers/AlignedVector.h"
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

	namespace model
	{

/*! \brief Polygon
 * \ingroup Model
 */
class T_DLLCLASS Polygon : public ISerializable
{
	T_RTTI_CLASS;

public:
	Polygon();

	Polygon(uint32_t material, uint32_t vertex1, uint32_t vertex2);

	Polygon(uint32_t material, uint32_t vertex1, uint32_t vertex2, uint32_t vertex3);

	Polygon(uint32_t material, uint32_t vertex1, uint32_t vertex2, uint32_t vertex3, uint32_t vertex4);

	void setMaterial(uint32_t material);

	uint32_t getMaterial() const;

	void setNormal(uint32_t normal);

	uint32_t getNormal() const;

	void setSmoothGroup(uint32_t smoothGroup);

	uint32_t getSmoothGroup() const;

	void clearVertices();

	void flipWinding();

	void addVertex(uint32_t vertex);

	void setVertex(uint32_t index, uint32_t vertex);

	uint32_t getVertex(uint32_t index) const;

	uint32_t getVertexCount() const;

	void setVertices(const AlignedVector< uint32_t >& vertices);

	const AlignedVector< uint32_t >& getVertices() const;

	AlignedVector< uint32_t >& getVertices();

	virtual void serialize(ISerializer& s) override final;

	bool operator == (const Polygon& r) const;

private:
	uint32_t m_material;
	uint32_t m_normal;
	uint32_t m_smoothGroup;
	AlignedVector< uint32_t > m_vertices;
};

	}
}
