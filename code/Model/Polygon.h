#ifndef traktor_model_Polygon_H
#define traktor_model_Polygon_H

#include <vector>
#include "Core/Config.h"
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
	namespace model
	{

/*! \brief Polygon
 * \ingroup Model
 */
class T_DLLCLASS Polygon
{
public:
	Polygon();

	Polygon(uint32_t material, uint32_t vertex1, uint32_t vertex2);

	Polygon(uint32_t material, uint32_t vertex1, uint32_t vertex2, uint32_t vertex3);

	Polygon(uint32_t material, uint32_t vertex1, uint32_t vertex2, uint32_t vertex3, uint32_t vertex4);

	void setMaterial(uint32_t material);

	uint32_t getMaterial() const;

	void setNormal(uint32_t normal);

	uint32_t getNormal() const;

	void clearVertices();

	void flipWinding();

	void addVertex(uint32_t vertex);

	void setVertex(uint32_t index, uint32_t vertex);

	uint32_t getVertex(uint32_t index) const;

	uint32_t getVertexCount() const;

	void setVertices(const std::vector< uint32_t >& vertices);

	const std::vector< uint32_t >& getVertices() const;

	bool operator == (const Polygon& r) const;

private:
	uint32_t m_material;
	uint32_t m_normal;
	std::vector< uint32_t > m_vertices;
};

	}
}

#endif	// traktor_model_Polygon_H
