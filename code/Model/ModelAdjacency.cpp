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

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.ModelAdjacency", ModelAdjacency, Object)

ModelAdjacency::ModelAdjacency(const Model* model, Mode mode, uint32_t channel)
:	m_model(model)
,	m_mode(mode)
,	m_channel(channel)
{
	for (uint32_t i = 0; i < model->getPolygonCount(); ++i)
		add(i);
}

ModelAdjacency::ModelAdjacency(const Model* model, const AlignedVector< uint32_t >& polygons, Mode mode, uint32_t channel)
:	m_model(model)
,	m_mode(mode)
,	m_channel(channel)
{
	for (auto polygon : polygons)
		add(polygon);
}

void ModelAdjacency::add(uint32_t polygon)
{
	const auto& polygonVertices = m_model->getPolygon(polygon).getVertices();
	for (uint32_t i = 0; i < polygonVertices.size(); ++i)
	{
		Edge e;
		e.polygon = polygon;
		e.index = i;
		m_edges.push_back(e);

		const uint32_t edge = uint32_t(m_edges.size()) - 1;

		uint32_t leftIndex0, leftIndex1;
		getEdgeIndices(edge, leftIndex0, leftIndex1);

		for (uint32_t j = 0; j < (uint32_t)m_edges.size(); ++j)
		{
			if (edge == j)
				continue;

			uint32_t rightIndex0, rightIndex1;
			getEdgeIndices(j, rightIndex0, rightIndex1);

			if (leftIndex0 == rightIndex1 && leftIndex1 == rightIndex0)
			{
				m_edges[j].share.push_back(edge);
				m_edges[edge].share.push_back(j);
			}
		}
	}
}

void ModelAdjacency::remove(uint32_t polygon)
{
	for (size_t i = 0; i < m_edges.size(); )
	{
		if (m_edges[i].polygon != polygon)
		{
			++i;
			continue;
		}

		for (size_t j = 0; j < m_edges.size(); ++j)
		{
			share_vector_t& edgeShare = m_edges[j].share;
			for (size_t k = 0; k < edgeShare.size(); )
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

		m_edges.erase(m_edges.begin() + i);
	}
}

void ModelAdjacency::update(uint32_t polygon)
{
	remove(polygon);
	add(polygon);
}

uint32_t ModelAdjacency::getEdge(uint32_t polygon, uint32_t polygonEdge) const
{
	for (uint32_t i = 0; i < m_edges.size(); ++i)
	{
		const Edge& edge = m_edges[i];
		if (edge.polygon == polygon && edge.index == polygonEdge)
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

		const Polygon& polygon = m_model->getPolygon(edge.polygon);
		const uint32_t vertexId1 = polygon.getVertex((edge.index + 1) % polygon.getVertexCount());

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

		const Polygon& polygon = m_model->getPolygon(edge.polygon);
		uint32_t vertexId0 = polygon.getVertex(edge.index);

		if (m_mode == Mode::ByVertex)
		{
			if (vertexId0 == vertexId)
				outLeavingEdges.push_back(i);
		}
		else if (m_mode == Mode::ByPosition)
		{
			uint32_t positionId0 = m_model->getVertex(vertexId0).getPosition();
			if (positionId0 == vertexId)
				outLeavingEdges.push_back(i);
		}
		else if (m_mode == Mode::ByTexCoord)
		{
			uint32_t texCoordId0 = m_model->getVertex(vertexId0).getTexCoord(m_channel);
			if (texCoordId0 == vertexId)
				outLeavingEdges.push_back(i);
		}
	}
}

void ModelAdjacency::getSharedEdges(uint32_t edge, share_vector_t& outSharedEdges) const
{
	outSharedEdges = m_edges[edge].share;
}

void ModelAdjacency::getSharedEdges(uint32_t polygon, uint32_t polygonEdge, share_vector_t& outSharedEdges) const
{
	outSharedEdges.resize(0);
	for (auto edge : m_edges)
	{
		if (edge.polygon == polygon && edge.index == polygonEdge)
		{
			outSharedEdges = edge.share;
			break;
		}
	}
}

uint32_t ModelAdjacency::getSharedEdgeCount(uint32_t edge) const
{
	return uint32_t(m_edges[edge].share.size());
}

uint32_t ModelAdjacency::getSharedEdgeCount(uint32_t polygon, uint32_t polygonEdge) const
{
	for (auto edge : m_edges)
	{
		if (edge.polygon == polygon && edge.index == polygonEdge)
			return uint32_t(edge.share.size());
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
	return uint32_t(m_edges.size());
}

void ModelAdjacency::getEdgeIndices(uint32_t edge, uint32_t& outIndex0, uint32_t& outIndex1) const
{
	const Edge& e = m_edges[edge];
	const Polygon& polygon = m_model->getPolygon(e.polygon);

	const auto& polygonVertices = polygon.getVertices();
	if (polygonVertices.size() < 2)
	{
		outIndex0 =
		outIndex1 = c_InvalidIndex;
	}

	uint32_t vertexId0 = polygonVertices[e.index];
	uint32_t vertexId1 = polygonVertices[(e.index + 1) % polygonVertices.size()];

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
}
