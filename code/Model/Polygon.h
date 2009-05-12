#ifndef traktor_model_Polygon_H
#define traktor_model_Polygon_H

#include "Core/Object.h"
#include "Model/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MODEL_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace model
	{

/*! \brief Polygon
 * \ingroup Model
 */
class T_DLLCLASS Polygon : public Object
{
	T_RTTI_CLASS(Polygon)

public:
	Polygon();

	void setMaterial(uint32_t material);

	uint32_t getMaterial() const;

	void clearVertices();

	void flipWinding();

	void addVertex(uint32_t vertex);

	void setVertex(uint32_t index, uint32_t vertex);

	uint32_t getVertex(uint32_t index) const;

	uint32_t getVertexCount() const;

	const std::vector< uint32_t >& getVertices() const;

	bool operator == (const Polygon& r) const;

private:
	int m_material;
	std::vector< uint32_t > m_vertices;
};

	}
}

#endif	// traktor_model_Polygon_H
