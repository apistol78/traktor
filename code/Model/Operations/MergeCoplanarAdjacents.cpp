#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Model/Model.h"
#include "Model/Operations/MergeCoplanarAdjacents.h"

namespace traktor
{
	namespace model
	{
		namespace
		{

struct Adjacency
{
	uint32_t left;
	uint32_t leftCount;
	uint32_t right;
	uint32_t rightCount;

	Adjacency()
	:	left(c_InvalidIndex)
	,	leftCount(0)
	,	right(c_InvalidIndex)
	,	rightCount(0)
	{
	}
};

struct VertexDirection
{
	Vector4 direction;
	bool have;
	bool curve;

	VertexDirection()
	:	direction(Vector4::zero())
	,	have(false)
	,	curve(false)
	{
	}
};

struct NoVerticesPred
{
	bool operator () (const Polygon& pol) const
	{
		return pol.getVertexCount() == 0;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.MergeCoplanarAdjacents", MergeCoplanarAdjacents, IModelOperation)

bool MergeCoplanarAdjacents::apply(Model& model) const
{
	std::vector< Polygon > polygons = model.getPolygons();

	// Calculate polygon normals.
	AlignedVector< Vector4 > normals(polygons.size(), Vector4::zero());
	for (uint32_t i = 0; i < uint32_t(polygons.size()); ++i)
	{
		const Polygon& polygon = polygons[i];
		if (polygon.getVertexCount() < 3)
			continue;

		const uint32_t baseIndex = 0;

		const std::vector< uint32_t >& vertices = polygon.getVertices();
		const Vertex* v[] =
		{
			&model.getVertex(vertices[baseIndex]),
			&model.getVertex(vertices[(baseIndex + 1) % vertices.size()]),
			&model.getVertex(vertices[(baseIndex + 2) % vertices.size()])
		};

		Vector4 p[] =
		{
			model.getPosition(v[0]->getPosition()),
			model.getPosition(v[1]->getPosition()),
			model.getPosition(v[2]->getPosition())
		};

		Vector4 ep[] = { p[2] - p[0], p[1] - p[0] };
		T_ASSERT (ep[0].length() > FUZZY_EPSILON);
		T_ASSERT (ep[1].length() > FUZZY_EPSILON);

		normals[i] = cross(ep[0], ep[1]).normalized();
	}

	// Build polygon adjacency map.
	std::map< uint64_t, Adjacency > adjacency;
	for (uint32_t i = 0; i < uint32_t(polygons.size()); ++i)
	{
		const Polygon& pol = polygons[i];
		const std::vector< uint32_t >& vertices = pol.getVertices();
		for (uint32_t j = 0; j < uint32_t(vertices.size()); ++j)
		{
			uint32_t v0 = vertices[j];
			uint32_t v1 = vertices[(j + 1) % uint32_t(vertices.size())];

			adjacency[uint64_t(v0) | (uint64_t(v1) << 32ULL)].left = i;
			adjacency[uint64_t(v0) | (uint64_t(v1) << 32ULL)].leftCount++;

			adjacency[uint64_t(v1) | (uint64_t(v0) << 32ULL)].right = i;
			adjacency[uint64_t(v1) | (uint64_t(v0) << 32ULL)].rightCount++;
		}
	}

	// Keep iterating until no more polygons are merged.
	for (;;)
	{
		uint32_t merged = 0;
		for (uint32_t i = 0; i < uint32_t(polygons.size()); ++i)
		{
			Polygon& polLeft = polygons[i];
		
			const std::vector< uint32_t >& vertices = polLeft.getVertices();

			uint32_t j = 0;
			while (j < uint32_t(vertices.size()))
			{
				uint32_t v0 = vertices[j];
				uint32_t v1 = vertices[(j + 1) % uint32_t(vertices.size())];

				const Adjacency adj = adjacency[uint64_t(v0) | (uint64_t(v1) << 32ULL)];
				if (adj.leftCount != 1 || adj.rightCount != 1 || adj.left == adj.right)
				{
					++j;
					continue;
				}

				if (dot3(normals[i], normals[adj.right]) < 1.0f - FUZZY_EPSILON)
				{
					++j;
					continue;
				}

				T_FATAL_ASSERT (adj.left == i);
				T_FATAL_ASSERT (adj.right != i);

				Polygon& polRight = polygons[adj.right];
				T_FATAL_ASSERT (polRight.getVertexCount() > 0);

				// Only merge with right if it's "simple"; ie doesn't have multiple adjacencies on any same edge.
				bool simple = true;
				for (uint32_t ii = 0; ii < polRight.getVertexCount(); ++ii)
				{
					uint32_t rv0 = polRight.getVertex(ii);
					uint32_t rv1 = polRight.getVertex((ii + 1) % polRight.getVertexCount());

					const Adjacency& adjr = adjacency[uint64_t(rv0) | (uint64_t(rv1) << 32ULL)];
					if (adjr.rightCount > 1)
					{
						simple = false;
						break;
					}
				}
				if (!simple)
				{
					++j;
					continue;
				}

				// Found merge pair; assimilate right into left.
				std::vector< uint32_t > verticesLeft = polLeft.getVertices();
				std::rotate(
					verticesLeft.begin(),
					std::find(verticesLeft.begin(), verticesLeft.end(), v1),
					verticesLeft.end()
				);
				T_FATAL_ASSERT (verticesLeft.back() == v0);

				std::vector< uint32_t > verticesRight = polRight.getVertices();
				std::rotate(
					verticesRight.begin(),
					std::find(verticesRight.begin(), verticesRight.end(), v0),
					verticesRight.end()
				);
				T_FATAL_ASSERT (verticesRight.back() == v1);

				std::vector< uint32_t > verticesMerged;
				verticesMerged.insert(verticesMerged.end(), verticesRight.begin(), verticesRight.end() - 1);
				verticesMerged.insert(verticesMerged.end(), verticesLeft.begin(), verticesLeft.end() - 1);
				T_ASSERT (verticesMerged.size() >= 3);

				// Save adjacency list in-case non-valid polygon.
				std::map< uint64_t, Adjacency > adjacencySave = adjacency;

				// Remove connecting edge from adjacency.
				adjacency.erase(uint64_t(v0) | (uint64_t(v1) << 32ULL));
				adjacency.erase(uint64_t(v1) | (uint64_t(v0) << 32ULL));

				for (std::map< uint64_t, Adjacency >::iterator ii = adjacency.begin(); ii != adjacency.end(); ++ii)
				{
					Adjacency& adj2 = ii->second;
					if (adj2.left == adj.right)
						adj2.left = adj.left;
					if (adj2.right == adj.right)
						adj2.right = adj.left;
				}

				// All adjacent edges which connect same polygon must be removed.
				for (uint32_t ii = 0; ii < uint32_t(verticesMerged.size()); )
				{
					uint32_t mv0 = verticesMerged[ii];
					uint32_t mv1 = verticesMerged[(ii + 1) % uint32_t(verticesMerged.size())];

					Adjacency& adj2 = adjacency[uint64_t(mv0) | (uint64_t(mv1) << 32ULL)];
					adj2.left = i;

					if (adj2.right == i)
					{
						T_FATAL_ASSERT (adj2.leftCount == 1);
						T_FATAL_ASSERT (adj2.rightCount == 1);

						verticesMerged.erase(verticesMerged.begin() + ii % uint32_t(verticesMerged.size()));
						verticesMerged.erase(verticesMerged.begin() + ii % uint32_t(verticesMerged.size()));

						adjacency.erase(uint64_t(mv0) | (uint64_t(mv1) << 32ULL));
						adjacency.erase(uint64_t(mv1) | (uint64_t(mv0) << 32ULL));

						ii = 0;
					}
					else
						++ii;
				}

				// Finally check if merged polygon is convex.
				bool convex = true;
				for (uint32_t ii = 0; ii < uint32_t(verticesMerged.size()); ++ii)
				{
					uint32_t mv0 = verticesMerged[ii];
					uint32_t mv1 = verticesMerged[(ii + 1) % uint32_t(verticesMerged.size())];
					uint32_t mv2 = verticesMerged[(ii + 2) % uint32_t(verticesMerged.size())];

					const Vector4& p0 = model.getVertexPosition(mv0);
					const Vector4& p1 = model.getVertexPosition(mv1);
					const Vector4& p2 = model.getVertexPosition(mv2);

					Vector4 n = cross(p0 - p1, p2 - p1).normalized();
					if (dot3(normals[i], n) < -FUZZY_EPSILON)
					{
						convex = false;
						break;
					}
				}

				if (!convex)
				{
					adjacency = adjacencySave;
					++j;
					continue;
				}

				// Finally update polygons.
				polLeft.setVertices(verticesMerged);
				polRight.setVertices(std::vector< uint32_t >());

				++merged;
				j = 0;
			}
		}

		if (!merged)
			break;
	}

	// Remove all polygons with no vertices.
	std::vector< Polygon >::iterator it = std::remove_if(polygons.begin(), polygons.end(), NoVerticesPred());
	polygons.erase(it, polygons.end());

	for (uint32_t i = 0; i < uint32_t(polygons.size()); ++i)
	{
		std::vector< uint32_t > vertices = polygons[i].getVertices();

		// Erase degenerate vertices.
		for (uint32_t j = 0; j < uint32_t(vertices.size()); )
		{
			const Vector4& p0 = model.getVertexPosition(vertices[j]);
			const Vector4& p1 = model.getVertexPosition(vertices[(j + 1) % vertices.size()]);
			const Vector4& p2 = model.getVertexPosition(vertices[(j + 2) % vertices.size()]);

			if (dot3((p1 - p0).normalized(), (p2 - p1).normalized()) >= 1.0f - FUZZY_EPSILON)
			{
				vertices.erase(vertices.begin() + (j + 1) % uint32_t(vertices.size()));
			}
			else
				++j;
		}

		polygons[i].setVertices(vertices);
	}

	model.setPolygons(polygons);
	return true;
}

	}
}
