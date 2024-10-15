#pragma optimize( "", off )

/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Model/Model.h"
#include "Model/ModelAdjacency.h"
#include "Model/Operations/Triangulate.h"
#include "Model/Test/CaseModelAdjacency.h"

namespace traktor::model::test
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.model.test.CaseModelAdjacency", 0, CaseModelAdjacency, traktor::test::Case)

void CaseModelAdjacency::run()
{
	typedef int32_t triangle_t[3];

	const Vector4 vertices[] =
	{
		Vector4( 0.0f,  0.0f, 0.0f),
		Vector4( 1.0f,  1.0f, 0.0f),
		Vector4( 1.0f,  0.0f, 0.0f),
		Vector4( 2.0f,  1.0f, 0.0f),
		Vector4( 2.0f,  0.0f, 0.0f)
	};

	const triangle_t triangles[] =
	{
		{ 0, 1, 2 },
		{ 2, 1, 3 },
		{ 3, 4, 2 }
	};

	Ref< Model > m = new Model();
	for (int32_t i = 0; i < 3; ++i)
	{
		uint32_t vi[3];
		for (int32_t j = 0; j < 3; ++j)
		{
			model::Vertex vx;
			vx.setPosition(m->addUniquePosition(vertices[triangles[i][j]]));
			vi[j] = m->addUniqueVertex(vx);
		}
		model::Polygon pol;
		pol.addVertex(vi[0]);
		pol.addVertex(vi[1]);
		pol.addVertex(vi[2]);
		m->addPolygon(pol);
	}
	CASE_ASSERT(m->getPolygonCount() == 3);

	{
		Ref< ModelAdjacency > ma = new ModelAdjacency(m, ModelAdjacency::Mode::ByPosition);
		CASE_ASSERT(ma->getEdgeCount() == 3 * 3);

		ModelAdjacency::share_vector_t edges;

		uint32_t i = ma->getEdge(0, 1);
		edges = ma->getSharedEdges(i);
		CASE_ASSERT(edges.size() == 1);

		uint32_t pn = ma->getPolygon(edges[0]);
		CASE_ASSERT(pn == 1);

		ma->remove(1, false);

		edges = ma->getSharedEdges(i);
		CASE_ASSERT(edges.size() == 0);

		ma->add(1);

		edges = ma->getSharedEdges(i);
		CASE_ASSERT(edges.size() == 1);

		pn = ma->getPolygon(edges[0]);
		CASE_ASSERT(pn == 1);
	}

	{
		Ref< ModelAdjacency > ma = new ModelAdjacency(m, ModelAdjacency::Mode::ByPosition);
		CASE_ASSERT(ma->getEdgeCount() == 3 * 3);
		ma->update(0);
		CASE_ASSERT(ma->getEdgeCount() == 3 * 3);
		checkSharingEdges(ma);
	}

	{
		Ref< ModelAdjacency > ma = new ModelAdjacency(m, ModelAdjacency::Mode::ByPosition);
		CASE_ASSERT(ma->getEdgeCount() == 3 * 3);
		ma->update(1);
		CASE_ASSERT(ma->getEdgeCount() == 3 * 3);
		checkSharingEdges(ma);
	}

	{
		Ref< ModelAdjacency > ma = new ModelAdjacency(m, ModelAdjacency::Mode::ByPosition);
		CASE_ASSERT(ma->getEdgeCount() == 3 * 3);
		ma->update(2);
		CASE_ASSERT(ma->getEdgeCount() == 3 * 3);
		checkSharingEdges(ma);
	}
}

void CaseModelAdjacency::checkSharingEdges(const ModelAdjacency* ma)
{
	for (uint32_t i = 0; i < ma->getEdgeCount(); ++i)
	{
		ModelAdjacency::share_vector_t shared = ma->getSharedEdges(i);

		for (const auto j : shared)
		{
			CASE_ASSERT(j != i);

			ModelAdjacency::share_vector_t other = ma->getSharedEdges(j);

			const auto it = std::find(other.begin(), other.end(), i);
			CASE_ASSERT(it != other.end());
		}
	}
}

}
