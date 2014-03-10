#ifndef traktor_model_ModelAdjacency_H
#define traktor_model_ModelAdjacency_H

#include "Core/Object.h"
#include "Core/Containers/StaticVector.h"

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

class Model;

class T_DLLCLASS ModelAdjacency : public Object
{
	T_RTTI_CLASS;

public:
	enum Mode
	{
		MdByVertex,
		MdByPosition
	};

	typedef StaticVector< uint32_t, 4 > share_vector_t;

	ModelAdjacency(const Model* model, Mode mode);

	ModelAdjacency(const Model* model, const std::vector< uint32_t >& polygons, Mode mode);

	/*! \brief Insert a new polygon into adjacency structure.
	 */
	void add(uint32_t polygon);

	/*! \brief Remove a polygon from adjacency structure.
	 */
	void remove(uint32_t polygon);

	/*! \brief Update polygon in adjacency structure.
	 */
	void update(uint32_t polygon);

	/*! \brief Return half edge from polygon edge.
	 */
	uint32_t getEdge(uint32_t polygon, uint32_t polygonEdge) const;

	/*! \brief Get edges "entering" given vertex.
	 */
	void getEnteringEdges(uint32_t vertexId, std::vector< uint32_t >& outEnteringEdges) const;

	/*! \brief Get edges "leaving" given vertex.
	 */
	void getLeavingEdges(uint32_t vertexId, std::vector< uint32_t >& outLeavingEdges) const;

	/*! \brief Get sharing edges; ie opposite edges.
	 */
	void getSharedEdges(uint32_t edge, std::vector< uint32_t >& outSharedEdges) const;

	/*! \brief Get sharing edges; ie opposite edges.
	 */
	void getSharedEdges(uint32_t polygon, uint32_t polygonEdge, std::vector< uint32_t >& outSharedEdges) const;

	/*! \brief Count number of sharing edges.
	 */
	uint32_t getSharedEdgeCount(uint32_t edge) const;

	/*! \brief Count number of sharing edges.
	 */
	uint32_t getSharedEdgeCount(uint32_t polygon, uint32_t polygonEdge) const;

	/*! \brief Get polygon owning edge.
	 */
	uint32_t getPolygon(uint32_t edge) const;

	/*! \brief Get polygon edge number from half-edge.
	 */
	uint32_t getPolygonEdge(uint32_t edge) const;

	/*! \brief Get total number of half-edges.
	 */
	uint32_t getEdgeCount() const;

	/*! \brief Get edge matching indices.
	 */
	void getEdgeIndices(uint32_t edge, uint32_t& outIndex0, uint32_t& outIndex1) const;

private:
	struct Edge
	{
		uint32_t polygon;
		uint32_t index;
		share_vector_t share;
	};

	Ref< const Model > m_model;
	Mode m_mode;
	std::vector< Edge > m_edges;
};

	}
}

#endif	// traktor_model_ModelAdjacency_H
