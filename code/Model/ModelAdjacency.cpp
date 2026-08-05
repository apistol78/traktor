/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Model/ModelAdjacency.h"

#include "Model/Model.h"

#include <algorithm>

namespace traktor::model
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.ModelAdjacency", ModelAdjacency, Object)

ModelAdjacency::ModelAdjacency(const Model* model, Mode mode, uint32_t channel)
	: m_model(model)
	, m_mode(mode)
	, m_channel(channel)
{
	m_polygonToFirstEdge.resize(model->getPolygonCount(), c_InvalidIndex);
	m_edgeLookup.resize(model->getVertexCount());
	for (uint32_t i = 0; i < model->getPolygonCount(); ++i)
		add(i);
}

ModelAdjacency::ModelAdjacency(const Model* model, const AlignedVector< uint32_t >& polygons, Mode mode, uint32_t channel)
	: m_model(model)
	, m_mode(mode)
	, m_channel(channel)
{
	m_polygonToFirstEdge.resize(polygons.size(), c_InvalidIndex);
	m_edgeLookup.resize(model->getVertexCount());
	for (auto polygon : polygons)
		add(polygon);
}

void ModelAdjacency::add(uint32_t polygon)
{
	T_FATAL_ASSERT(polygon < m_model->getPolygonCount());
	T_FATAL_ASSERT(m_polygonToFirstEdge[polygon] == c_InvalidIndex);

	const auto& polygonVertices = m_model->getPolygon(polygon).getVertices();
	share_vector_t opposite;
	for (uint32_t i = 0; i < polygonVertices.size(); ++i)
	{
		Edge& e = m_edges.push_back();
		e.polygon = polygon;
		e.polygonEdge = i;

		const uint32_t edge = (uint32_t)(m_edges.size() - 1);
		getEdgeIndices(edge, e.index0, e.index1);

		// Link with all existing opposite half-edges (those running index1 -> index0).
		collectOppositeEdges(edge, opposite);
		for (uint32_t j : opposite)
		{
			shareDataPushBack(m_edges[j], edge);
			shareDataPushBack(e, j);
		}
		edgeLookupInsert(edge);

		// Remember offset to first edge of polyogn.
		if (m_polygonToFirstEdge[polygon] == c_InvalidIndex)
			m_polygonToFirstEdge[polygon] = edge;
	}
}

void ModelAdjacency::remove(uint32_t polygon, bool reindex)
{
	const uint32_t firstEdge = m_polygonToFirstEdge[polygon];
	T_FATAL_ASSERT(firstEdge != c_InvalidIndex);

	for (uint32_t i = firstEdge; i < m_edges.size() && m_edges[i].polygon == polygon; ++i)
	{
		Edge& edge = m_edges[i];

		// Drop from the opposite-edge lookup while index0 is still valid.
		edgeLookupErase(i);

		// Remove references to this edge from sharing edges.
		for (uint32_t o = 0; o < edge.shareDataCount; ++o)
		{
			const uint32_t edgeIndex = m_shareData[edge.shareDataOffset + o];
			Edge& edgeShare = m_edges[edgeIndex];

			for (uint32_t k = 0; k < edgeShare.shareDataCount; ++k)
			{
				const uint32_t shareEdgeIndex = m_shareData[edgeShare.shareDataOffset + k];
				if (shareEdgeIndex == i)
				{
					shareDataErase(edgeShare, k);
					break;
				}
			}
		}

		edge.polygon = c_InvalidIndex;
		edge.index0 = c_InvalidIndex;
		edge.index1 = c_InvalidIndex;
		edge.shareDataCount = 0;
	}

	m_polygonToFirstEdge[polygon] = c_InvalidIndex;

	if (reindex)
	{
		for (auto& edge : m_edges)
			if (edge.polygon != c_InvalidIndex && edge.polygon > polygon)
				edge.polygon--;

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
	for (uint32_t i = firstEdge; i < m_edges.size() && m_edges[i].polygon == polygon; ++i)
	{
		Edge& edge = m_edges[i];

		// Drop from the opposite-edge lookup at its current (pre-update) index0.
		edgeLookupErase(i);

		for (uint32_t o = 0; o < edge.shareDataCount; ++o)
		{
			const uint32_t edgeIndex = m_shareData[edge.shareDataOffset + o];
			Edge& edgeShare = m_edges[edgeIndex];

			for (uint32_t k = 0; k < edgeShare.shareDataCount; ++k)
			{
				const uint32_t shareEdgeIndex = m_shareData[edgeShare.shareDataOffset + k];
				if (shareEdgeIndex == i)
				{
					shareDataErase(edgeShare, k);
					break;
				}
			}
		}

		edge.shareDataCount = 0;
	}

	// Add sharing references.
	share_vector_t opposite;
	const auto& polygonVertices = m_model->getPolygon(polygon).getVertices();
	uint32_t edgeIndex = firstEdge;
	for (uint32_t i = 0; i < polygonVertices.size(); ++i)
	{
		Edge& e = m_edges[edgeIndex];
		T_FATAL_ASSERT(e.polygon == polygon);
		T_FATAL_ASSERT(e.polygonEdge == i);

		getEdgeIndices(edgeIndex, e.index0, e.index1);

		// Link with the current opposite half-edges (index1 -> index0) via the lookup, then
		// re-register this edge under its new index0.
		collectOppositeEdges(edgeIndex, opposite);
		for (uint32_t j : opposite)
		{
			shareDataPushBack(m_edges[j], edgeIndex);
			shareDataPushBack(e, j);
		}
		edgeLookupInsert(edgeIndex);

		++edgeIndex;
	}
}

uint32_t ModelAdjacency::getEdge(uint32_t polygon, uint32_t polygonEdge) const
{
	const uint32_t firstEdge = m_polygonToFirstEdge[polygon];
	T_FATAL_ASSERT(firstEdge != c_InvalidIndex);

	for (uint32_t i = firstEdge; i < m_edges.size() && m_edges[i].polygon == polygon; ++i)
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

ModelAdjacency::ShareView ModelAdjacency::getSharedEdges(uint32_t edge) const
{
	return { m_edges[edge].shareDataCount, &m_shareData[m_edges[edge].shareDataOffset] };
}

ModelAdjacency::ShareView ModelAdjacency::getSharedEdges(uint32_t polygon, uint32_t polygonEdge) const
{
	const uint32_t firstEdge = m_polygonToFirstEdge[polygon];
	T_FATAL_ASSERT(firstEdge != c_InvalidIndex);

	for (uint32_t i = firstEdge; i < m_edges.size() && m_edges[i].polygon == polygon; ++i)
	{
		const Edge& edge = m_edges[i];
		if (edge.polygon == polygon && edge.polygonEdge == polygonEdge)
			return getSharedEdges(i);
	}

	return { 0, nullptr };
}

uint32_t ModelAdjacency::getSharedEdgeCount(uint32_t edge) const
{
	return m_edges[edge].shareDataCount;
}

uint32_t ModelAdjacency::getSharedEdgeCount(uint32_t polygon, uint32_t polygonEdge) const
{
	const uint32_t firstEdge = m_polygonToFirstEdge[polygon];
	T_FATAL_ASSERT(firstEdge != c_InvalidIndex);

	for (uint32_t i = firstEdge; i < m_edges.size() && m_edges[i].polygon == polygon; ++i)
	{
		const Edge& edge = m_edges[i];
		if (edge.polygon == polygon && edge.polygonEdge == polygonEdge)
			return edge.shareDataCount;
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

void ModelAdjacency::shareDataPushBack(Edge& edge, uint32_t value)
{
	if (edge.shareDataCount >= edge.shareDataCapacity)
	{
		// Region full; move all to the end before appending.
		edge.shareDataCapacity += 16;
		const uint32_t newOffset = m_shareData.size();

		// Reserve the whole appended region up front. The copy loop below reads from
		// m_shareData while pushing into it, so a reallocation mid-loop would leave the
		// source (an argument reference into the old, freed buffer) dangling - undefined
		// behaviour that read back differently on MSVC vs GCC and made adjacency, and thus
		// the Reduce operation, diverge between platforms. Reserving the full count keeps
		// every source reference valid for the duration of the loops.
		m_shareData.reserve(newOffset + edge.shareDataCapacity);
		for (uint32_t i = 0; i < edge.shareDataCount; ++i)
			m_shareData.push_back(m_shareData[edge.shareDataOffset + i]);
		for (uint32_t i = edge.shareDataCount; i < edge.shareDataCapacity; ++i)
			m_shareData.push_back(0);

		edge.shareDataOffset = newOffset;
	}

	m_shareData[edge.shareDataOffset + edge.shareDataCount] = value;
	edge.shareDataCount++;
}

void ModelAdjacency::shareDataErase(Edge& edge, uint32_t index)
{
	T_FATAL_ASSERT(edge.shareDataCount > 0);

	for (uint32_t i = index; i < edge.shareDataCount - 1; ++i)
		m_shareData[edge.shareDataOffset + i] = m_shareData[edge.shareDataOffset + i + 1];

	edge.shareDataCount--;
}

void ModelAdjacency::edgeLookupInsert(uint32_t edge)
{
	const uint32_t index0 = m_edges[edge].index0;
	if (index0 == c_InvalidIndex)
		return;
	if (index0 >= (uint32_t)m_edgeLookup.size())
		m_edgeLookup.resize(index0 + 1);
	m_edgeLookup[index0].push_back(edge);
}

void ModelAdjacency::edgeLookupErase(uint32_t edge)
{
	const uint32_t index0 = m_edges[edge].index0;
	if (index0 == c_InvalidIndex || index0 >= (uint32_t)m_edgeLookup.size())
		return;

	AlignedVector< uint32_t >& bucket = m_edgeLookup[index0];
	for (uint32_t i = 0; i < (uint32_t)bucket.size(); ++i)
	{
		if (bucket[i] == edge)
		{
			bucket[i] = bucket.back();
			bucket.pop_back();
			return;
		}
	}
}

void ModelAdjacency::collectOppositeEdges(uint32_t edge, share_vector_t& outOpposite) const
{
	outOpposite.resize(0);

	const uint32_t index0 = m_edges[edge].index0;
	const uint32_t index1 = m_edges[edge].index1;
	if (index0 == c_InvalidIndex || index1 == c_InvalidIndex)
		return;

	if (index1 < (uint32_t)m_edgeLookup.size())
	{
		for (uint32_t candidate : m_edgeLookup[index1])
			if (m_edges[candidate].index1 == index0)
				outOpposite.push_back(candidate);
	}

	std::sort(outOpposite.begin(), outOpposite.end());
}

}
