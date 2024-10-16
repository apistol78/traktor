/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Model/Model.h"
#include "Model/ModelAdjacency.h"

namespace traktor::model
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.ModelAdjacency", ModelAdjacency, Object)

ModelAdjacency::ModelAdjacency(const Model* model, Mode mode, uint32_t channel)
:	m_model(model)
,	m_mode(mode)
,	m_channel(channel)
{
	m_polygonToFirstEdge.resize(model->getPolygonCount(), c_InvalidIndex);
	for (uint32_t i = 0; i < model->getPolygonCount(); ++i)
		add(i);
}

ModelAdjacency::ModelAdjacency(const Model* model, const AlignedVector< uint32_t >& polygons, Mode mode, uint32_t channel)
:	m_model(model)
,	m_mode(mode)
,	m_channel(channel)
{
	m_polygonToFirstEdge.resize(polygons.size(), c_InvalidIndex);
	for (auto polygon : polygons)
		add(polygon);
}

void ModelAdjacency::add(uint32_t polygon)
{
	T_FATAL_ASSERT(polygon < m_model->getPolygonCount());
	T_FATAL_ASSERT(m_polygonToFirstEdge[polygon] == c_InvalidIndex);

	const auto& polygonVertices = m_model->getPolygon(polygon).getVertices();
	for (uint32_t i = 0; i < polygonVertices.size(); ++i)
	{
		Edge& e = m_edges.push_back();
		e.polygon = polygon;
		e.polygonEdge = i;

		const uint32_t edge = (uint32_t)(m_edges.size() - 1);
		getEdgeIndices(edge, e.index0, e.index1);

		// Link with all other existing edges.
		for (uint32_t j = 0; j < edge; ++j)
		{
			Edge& rightEdge = m_edges[j];
			if (e.index0 == rightEdge.index1 && e.index1 == rightEdge.index0)
			{
				rightEdge.share.push_back(edge);
				e.share.push_back(j);
			}
		}

		// Remember offset to first edge of polyogn.
		if (m_polygonToFirstEdge[polygon] == c_InvalidIndex)
			m_polygonToFirstEdge[polygon] = edge;
	}
}

void ModelAdjacency::remove(uint32_t polygon, bool reindex)
{
	const uint32_t firstEdge = m_polygonToFirstEdge[polygon];
	T_FATAL_ASSERT(firstEdge != c_InvalidIndex);

	for (uint32_t i = firstEdge; m_edges[i].polygon == polygon; ++i)
	{
		Edge& edge = m_edges[i];

		// Remove references to this edge from sharing edges.
		for (uint32_t j : edge.share)
		{
			Edge& edgeShare = m_edges[j];
			for (auto it = edgeShare.share.begin(); it != edgeShare.share.end(); ++it)
			{
				if (*it == i)
				{
					edgeShare.share.erase(it);
					break;
				}
			}
		}

		edge.polygon = c_InvalidIndex;
		edge.index0 = c_InvalidIndex;
		edge.index1 = c_InvalidIndex;
		edge.share.resize(0);
	}

	m_polygonToFirstEdge[polygon] = c_InvalidIndex;

	if (reindex)
	{
		for (auto& edge : m_edges)
		{
			if (edge.polygon != c_InvalidIndex && edge.polygon > polygon)
				edge.polygon--;
		}

		m_polygonToFirstEdge.erase(m_polygonToFirstEdge.begin() + polygon);
	}
}

void ModelAdjacency::update(uint32_t polygon)
{
	T_FATAL_ASSERT(polygon < m_model->getPolygonCount());

	// #note Assuming same number of edges.

	const uint32_t firstEdge = m_polygonToFirstEdge[polygon];
	T_FATAL_ASSERT(firstEdge != c_InvalidIndex);

	// Remove references to this polygon's edges from sharing edges.
	for (uint32_t i = firstEdge; m_edges[i].polygon == polygon; ++i)
	{
		Edge& edge = m_edges[i];
		for (uint32_t j : edge.share)
		{
			Edge& edgeShare = m_edges[j];
			for (auto it = edgeShare.share.begin(); it != edgeShare.share.end(); ++it)
			{
				if (*it == i)
				{
					edgeShare.share.erase(it);
					break;
				}
			}
		}
		edge.share.resize(0);
	}

	// Add sharing references.
	const auto& polygonVertices = m_model->getPolygon(polygon).getVertices();
	uint32_t edgeIndex = firstEdge;
	for (uint32_t i = 0; i < polygonVertices.size(); ++i)
	{
		Edge& e = m_edges[edgeIndex];
		T_FATAL_ASSERT(e.polygon == polygon);
		T_FATAL_ASSERT(e.polygonEdge == i);

		getEdgeIndices(edgeIndex, e.index0, e.index1);

		for (uint32_t j = 0; j < (uint32_t)m_edges.size(); ++j)
		{
			Edge& rightEdge = m_edges[j];
			if (e.index0 == rightEdge.index1 && e.index1 == rightEdge.index0)
			{
				rightEdge.share.push_back(edgeIndex);
				e.share.push_back(j);
			}
		}
		++edgeIndex;
	}
}

uint32_t ModelAdjacency::getEdge(uint32_t polygon, uint32_t polygonEdge) const
{
	const uint32_t firstEdge = m_polygonToFirstEdge[polygon];
	T_FATAL_ASSERT(firstEdge != c_InvalidIndex);

	for (uint32_t i = firstEdge; m_edges[i].polygon == polygon; ++i)
	{
		const Edge& edge = m_edges[i];
		if (edge.polygon == polygon && edge.polygonEdge == polygonEdge)
			return i;
	}

	return c_InvalidIndex;
}

void ModelAdjacency::getEnteringEdges(uint32_t vertexId, share_vector_t& outEnteringEdges) const
{
	if (m_mode == Mode::ByPosition)
		vertexId = m_model->getVertex(vertexId).getPosition();
	else if (m_mode == Mode::ByTexCoord)
		vertexId = m_model->getVertex(vertexId).getTexCoord(m_channel);

	outEnteringEdges.resize(0);
	for (uint32_t i = 0; i < m_edges.size(); ++i)
	{
		const Edge& edge = m_edges[i];
		if (edge.polygon == c_InvalidIndex)
			continue;

		const Polygon& polygon = m_model->getPolygon(edge.polygon);
		const uint32_t vertexId1 = polygon.getVertex((edge.polygonEdge + 1) % polygon.getVertexCount());

		if (m_mode == Mode::ByVertex)
		{
			if (vertexId1 == vertexId)
				outEnteringEdges.push_back(i);
		}
		else if (m_mode == Mode::ByPosition)
		{
			const uint32_t positionId1 = m_model->getVertex(vertexId1).getPosition();
			if (positionId1 == vertexId)
				outEnteringEdges.push_back(i);
		}
		else if (m_mode == Mode::ByTexCoord)
		{
			const uint32_t texCoordId1 = m_model->getVertex(vertexId1).getTexCoord(m_channel);
			if (texCoordId1 == vertexId)
				outEnteringEdges.push_back(i);
		}
	}
}

void ModelAdjacency::getLeavingEdges(uint32_t vertexId, share_vector_t& outLeavingEdges) const
{
	if (m_mode == Mode::ByPosition)
		vertexId = m_model->getVertex(vertexId).getPosition();
	else if (m_mode == Mode::ByTexCoord)
		vertexId = m_model->getVertex(vertexId).getTexCoord(m_channel);

	outLeavingEdges.resize(0);
	for (uint32_t i = 0; i < m_edges.size(); ++i)
	{
		const Edge& edge = m_edges[i];
		if (edge.polygon == c_InvalidIndex)
			continue;

		const Polygon& polygon = m_model->getPolygon(edge.polygon);
		const uint32_t vertexId0 = polygon.getVertex(edge.polygonEdge);

		if (m_mode == Mode::ByVertex)
		{
			if (vertexId0 == vertexId)
				outLeavingEdges.push_back(i);
		}
		else if (m_mode == Mode::ByPosition)
		{
			const uint32_t positionId0 = m_model->getVertex(vertexId0).getPosition();
			if (positionId0 == vertexId)
				outLeavingEdges.push_back(i);
		}
		else if (m_mode == Mode::ByTexCoord)
		{
			const uint32_t texCoordId0 = m_model->getVertex(vertexId0).getTexCoord(m_channel);
			if (texCoordId0 == vertexId)
				outLeavingEdges.push_back(i);
		}
	}
}

const ModelAdjacency::share_vector_t& ModelAdjacency::getSharedEdges(uint32_t edge) const
{
	return m_edges[edge].share;
}

ModelAdjacency::share_vector_t ModelAdjacency::getSharedEdges(uint32_t polygon, uint32_t polygonEdge) const
{
	const uint32_t firstEdge = m_polygonToFirstEdge[polygon];
	T_FATAL_ASSERT(firstEdge != c_InvalidIndex);

	for (uint32_t i = firstEdge; m_edges[i].polygon == polygon; ++i)
	{
		const Edge& edge = m_edges[i];
		if (edge.polygon == polygon && edge.polygonEdge == polygonEdge)
			return edge.share;
	}

	return share_vector_t();
}

uint32_t ModelAdjacency::getSharedEdgeCount(uint32_t edge) const
{
	return (uint32_t)m_edges[edge].share.size();
}

uint32_t ModelAdjacency::getSharedEdgeCount(uint32_t polygon, uint32_t polygonEdge) const
{
	const uint32_t firstEdge = m_polygonToFirstEdge[polygon];
	T_FATAL_ASSERT(firstEdge != c_InvalidIndex);

	for (uint32_t i = firstEdge; m_edges[i].polygon == polygon; ++i)
	{
		const Edge& edge = m_edges[i];
		if (edge.polygon == polygon && edge.polygonEdge == polygonEdge)
			return (uint32_t)edge.share.size();
	}

	return 0;
}

uint32_t ModelAdjacency::getPolygon(uint32_t edge) const
{
	return m_edges[edge].polygon;
}

uint32_t ModelAdjacency::getPolygonEdge(uint32_t edge) const
{
	return m_edges[edge].polygonEdge;
}

uint32_t ModelAdjacency::getEdgeCount() const
{
	return uint32_t(m_edges.size());
}

void ModelAdjacency::getEdgeIndices(uint32_t edge, uint32_t& outIndex0, uint32_t& outIndex1) const
{
	const Edge& e = m_edges[edge];
	if (e.polygon == c_InvalidIndex)
	{
		outIndex0 =
		outIndex1 = c_InvalidIndex;
		return;
	}

	const Polygon& polygon = m_model->getPolygon(e.polygon);
	const Polygon::vertices_t& polygonVertices = polygon.getVertices();

	if (polygonVertices.size() < 2)
	{
		outIndex0 =
		outIndex1 = c_InvalidIndex;
		return;
	}

	const uint32_t vertexId0 = polygonVertices[e.polygonEdge];
	const uint32_t vertexId1 = polygonVertices[(e.polygonEdge + 1) % polygonVertices.size()];

	switch (m_mode)
	{
	default:
	case Mode::ByVertex:
		{
			outIndex0 = vertexId0;
			outIndex1 = vertexId1;
		}
		break;

	case Mode::ByPosition:
		{
			const Vertex& vertex0 = m_model->getVertex(vertexId0);
			const Vertex& vertex1 = m_model->getVertex(vertexId1);
			outIndex0 = vertex0.getPosition();
			outIndex1 = vertex1.getPosition();
		}
		break;

	case Mode::ByTexCoord:
		{
			const Vertex& vertex0 = m_model->getVertex(vertexId0);
			const Vertex& vertex1 = m_model->getVertex(vertexId1);
			outIndex0 = vertex0.getTexCoord(m_channel);
			outIndex1 = vertex1.getTexCoord(m_channel);
		}
		break;
	}
}

}
