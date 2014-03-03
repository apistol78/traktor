#include "Model/Model.h"
#include "Model/ModelAdjacency.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.ModelAdjacency", ModelAdjacency, Object)

ModelAdjacency::ModelAdjacency(const Model* model, Mode mode)
:	m_model(model)
,	m_mode(mode)
{
	for (uint32_t i = 0; i < model->getPolygonCount(); ++i)
		add(i);
}

ModelAdjacency::~ModelAdjacency()
{
	for (uint32_t i = 0; i < m_edges.size(); ++i)
		delete m_edges[i].share;
}

void ModelAdjacency::add(uint32_t polygon)
{
	const std::vector< uint32_t >& polygonVertices = m_model->getPolygon(polygon).getVertices();
	for (uint32_t i = 0; i < polygonVertices.size(); ++i)
	{
		Edge e;
		e.polygon = polygon;
		e.index = i;
		e.share = new std::vector< uint32_t >();
		m_edges.push_back(e);

		uint32_t edge = m_edges.size() - 1;

		uint32_t leftIndex0, leftIndex1;
		getMatchIndices(m_edges[edge], leftIndex0, leftIndex1);

		for (uint32_t j = 0; j < m_edges.size(); ++j)
		{
			if (edge == j)
				continue;

			uint32_t rightIndex0, rightIndex1;
			getMatchIndices(m_edges[j], rightIndex0, rightIndex1);

			if (leftIndex0 == rightIndex1 && leftIndex1 == rightIndex0)
			{
				m_edges[j].share->push_back(edge);
				m_edges[edge].share->push_back(j);
			}
		}
	}
}

void ModelAdjacency::remove(uint32_t polygon)
{
	for (uint32_t i = 0; i < m_edges.size(); )
	{
		if (m_edges[i].polygon != polygon)
		{
			++i;
			continue;
		}

		for (uint32_t j = 0; j < m_edges.size(); ++j)
		{
			std::vector< uint32_t >& edgeShare = *m_edges[j].share;
			for (uint32_t k = 0; k < edgeShare.size(); )
			{
				if (edgeShare[k] == i)
					edgeShare.erase(edgeShare.begin() + k);
				else 
				{
					if (edgeShare[k] > i)
						edgeShare[k]--;
					++k;
				}
			}
		}

		delete m_edges[i].share;
		m_edges.erase(m_edges.begin() + i);
	}
}

void ModelAdjacency::update(uint32_t polygon)
{
	remove(polygon);
	add(polygon);
}

void ModelAdjacency::getEnteringEdges(uint32_t vertexId, std::vector< uint32_t >& outEnteringEdges) const
{
	if (m_mode == MdByPosition)
		vertexId = m_model->getVertex(vertexId).getPosition();

	outEnteringEdges.resize(0);
	for (uint32_t i = 0; i < m_edges.size(); ++i)
	{
		const Edge& edge = m_edges[i];

		const Polygon& polygon = m_model->getPolygon(edge.polygon);
		uint32_t vertexId1 = polygon.getVertex((edge.index + 1) % polygon.getVertexCount());

		if (m_mode == MdByVertex)
		{
			if (vertexId1 == vertexId)
				outEnteringEdges.push_back(i);
		}
		else if (m_mode == MdByPosition)
		{
			uint32_t positionId1 = m_model->getVertex(vertexId1).getPosition();
			if (positionId1 == vertexId)
				outEnteringEdges.push_back(i);
		}
	}
}

void ModelAdjacency::getLeavingEdges(uint32_t vertexId, std::vector< uint32_t >& outLeavingEdges) const
{
	if (m_mode == MdByPosition)
		vertexId = m_model->getVertex(vertexId).getPosition();

	outLeavingEdges.resize(0);
	for (uint32_t i = 0; i < m_edges.size(); ++i)
	{
		const Edge& edge = m_edges[i];

		const Polygon& polygon = m_model->getPolygon(edge.polygon);
		uint32_t vertexId0 = polygon.getVertex(edge.index);

		if (m_mode == MdByVertex)
		{
			if (vertexId0 == vertexId)
				outLeavingEdges.push_back(i);
		}
		else if (m_mode == MdByPosition)
		{
			uint32_t positionId0 = m_model->getVertex(vertexId0).getPosition();
			if (positionId0 == vertexId)
				outLeavingEdges.push_back(i);
		}
	}
}

void ModelAdjacency::getSharedEdges(uint32_t edge, std::vector< uint32_t >& outSharedEdges) const
{
	if (m_edges[edge].share)
		outSharedEdges = *(m_edges[edge].share);
	else
		outSharedEdges.resize(0);
}

void ModelAdjacency::getSharedEdges(uint32_t polygon, uint32_t polygonEdge, std::vector< uint32_t >& outSharedEdges) const
{
	outSharedEdges.resize(0);
	for (std::vector< Edge >::const_iterator i = m_edges.begin(); i != m_edges.end(); ++i)
	{
		if (i->polygon == polygon && i->index == polygonEdge)
		{
			outSharedEdges = *(i->share);
			break;
		}
	}
}

uint32_t ModelAdjacency::getSharedEdgeCount(uint32_t edge) const
{
	if (m_edges[edge].share)
		return m_edges[edge].share->size();
	else
		return 0;
}

uint32_t ModelAdjacency::getSharedEdgeCount(uint32_t polygon, uint32_t polygonEdge) const
{
	for (std::vector< Edge >::const_iterator i = m_edges.begin(); i != m_edges.end(); ++i)
	{
		if (i->polygon == polygon && i->index == polygonEdge)
			return i->share->size();
	}
	return 0;
}

uint32_t ModelAdjacency::getPolygon(uint32_t edge) const
{
	return m_edges[edge].polygon;
}

uint32_t ModelAdjacency::getPolygonEdge(uint32_t edge) const
{
	return m_edges[edge].index;
}

uint32_t ModelAdjacency::getEdgeCount() const
{
	return m_edges.size();
}

void ModelAdjacency::getMatchIndices(const Edge& edge, uint32_t& outIndex0, uint32_t& outIndex1) const
{
	const Polygon& polygon = m_model->getPolygon(edge.polygon);
	const std::vector< uint32_t >& polygonVertices = polygon.getVertices();

	uint32_t vertexId0 = polygonVertices[edge.index];
	uint32_t vertexId1 = polygonVertices[(edge.index + 1) % polygonVertices.size()];

	switch (m_mode)
	{
	default:
	case MdByVertex:
		{
			outIndex0 = vertexId0;
			outIndex1 = vertexId1;
		}
		break;

	case MdByPosition:
		{
			const Vertex& vertex0 = m_model->getVertex(vertexId0);
			const Vertex& vertex1 = m_model->getVertex(vertexId1);
			outIndex0 = vertex0.getPosition();
			outIndex1 = vertex1.getPosition();
		}
		break;
	}
}

	}
}
