#ifndef traktor_model_ModelAdjacency_H
#define traktor_model_ModelAdjacency_H

#include "Core/Object.h"

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

	ModelAdjacency(const Model* model, Mode mode);

	virtual ~ModelAdjacency();

	void add(uint32_t polygon);

	void remove(uint32_t polygon);

	void getSharedEdges(uint32_t polygon, uint32_t edge, std::vector< uint32_t >& outSharedEdges) const;

	uint32_t getPolygon(uint32_t edge) const;

	uint32_t getPolygonEdge(uint32_t edge) const;

	void removeEdge(uint32_t edge);

private:
	struct Edge
	{
		uint32_t polygon;
		uint32_t index;
		std::vector< uint32_t >* share;
	};

	Ref< const Model > m_model;
	Mode m_mode;
	std::vector< Edge > m_edges;

	void getMatchIndices(const Edge& edge, uint32_t& outIndex0, uint32_t& outIndex1) const;
};

	}
}

#endif	// traktor_model_ModelAdjacency_H
