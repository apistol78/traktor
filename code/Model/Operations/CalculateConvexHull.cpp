#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Plane.h"
#include "Model/Model.h"
#include "Model/Operations/CalculateConvexHull.h"

namespace traktor
{
	namespace model
	{
		namespace
		{

struct HullFace
{
	uint32_t i[3];

	HullFace()
	{
		i[0] = ~0U; i[1] = ~0U; i[2] = ~0U;
	}

	HullFace(
		uint32_t i1, uint32_t i2, uint32_t i3
	)
	{
		i[0] = i1; i[1] = i2; i[2] = i3;
	}
};

struct HullFaceAdjacency
{
	uint32_t n[3];

	HullFaceAdjacency()
	{
		n[0] = ~0U; n[1] = ~0U; n[2] = ~0U;
	}

	HullFaceAdjacency(
		uint32_t n1, uint32_t n2, uint32_t n3
	)
	{
		n[0] = n1; n[1] = n2; n[2] = n3;
	}
};

uint32_t calculateAdjacency(const std::vector< HullFace >& faces, std::vector< HullFaceAdjacency >& outAdjacency)
{
	uint32_t errorCount = 0;
	for (uint32_t i = 0; i < uint32_t(faces.size()); ++i)
	{
		HullFaceAdjacency adj;

		for (uint32_t j = 0; j < 3; ++j)
		{
			int a1 = faces[i].i[j];
			int a2 = faces[i].i[(j + 1) % 3];

			for (uint32_t k = 0; k < uint32_t(faces.size()) && adj.n[j] == ~0U; ++k)
			{
				if (i == k)
					continue;

				for (uint32_t m = 0; m < 3; ++m)
				{
					int na1 = faces[k].i[m];
					int na2 = faces[k].i[(m + 1) % 3];

					T_ASSERT_M(!(a1 == na1 && a2 == na2), L"Adjacent face with different winding");

					if (a1 == na2 && a2 == na1)
					{
						adj.n[j] = k;
						break;
					}
				}
			}
		}

		if (adj.n[0] == ~0U || adj.n[1] == ~0U || adj.n[2] == ~0U)
			++errorCount;

		outAdjacency.push_back(adj);
	}
	return errorCount;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.CalculateConvexHull", CalculateConvexHull, IModelOperation)

bool CalculateConvexHull::apply(Model& model) const
{
	uint32_t errorCount = 0;

	AlignedVector< Vector4 > vertices = model.getPositions();

	// Use first polygon as basis for tetrahedron.
	Polygon p0 = model.getPolygons().front();

	uint32_t i0 = model.getVertex(p0.getVertex(0)).getPosition();
	uint32_t i1 = model.getVertex(p0.getVertex(1)).getPosition();
	uint32_t i2 = model.getVertex(p0.getVertex(2)).getPosition();

	Plane pll(
		vertices[i0],
		vertices[i1],
		vertices[i2]
	);

	// Build initial tetrahedron.
	uint32_t it = ~0U;
	for (uint32_t i = 0; i < uint32_t(vertices.size()); ++i)
	{
		if (i == i0 || i == i1 || i == i2)
			continue;

		Scalar d = pll.distance(vertices[i]);
		if (d < -FUZZY_EPSILON)
		{
			it = i;
			break;
		}
	}
	if (it == ~0U)
		return false;

	std::vector< HullFace > faces;
	faces.reserve(128);
	faces.push_back(HullFace(i0, i1, i2));
	faces.push_back(HullFace(i1, i0, it));
	faces.push_back(HullFace(i2, i1, it));
	faces.push_back(HullFace(i0, i2, it));

	std::vector< HullFaceAdjacency > adjacency;
	errorCount = calculateAdjacency(faces, adjacency);

	if (errorCount > 0)
		T_DEBUG(errorCount << L" adjanceny error(s)");

	std::vector< bool > visible;
	std::vector< std::pair< uint32_t, uint32_t > > silouette;

	for (uint32_t i = 0; i < uint32_t(vertices.size()); ++i)
	{
		visible.resize(faces.size());
		for (uint32_t j = 0; j < uint32_t(faces.size()); ++j)
		{
			Plane pl(
				vertices[faces[j].i[0]],
				vertices[faces[j].i[1]],
				vertices[faces[j].i[2]]
			);
			visible[j] = bool(pl.distance(vertices[i]) >= -FUZZY_EPSILON);
		}

		silouette.resize(0);
		for (uint32_t j = 0; j < uint32_t(faces.size()); ++j)
		{
			for (uint32_t k = 0; k < 3; ++k)
			{
				uint32_t n = adjacency[j].n[k];
				if (n != uint32_t(~0UL) && !visible[j] && visible[n])
				{
					silouette.push_back(std::make_pair(
						faces[j].i[k],
						faces[j].i[(k + 1) % 3]
					));
				}
			}
		}
		if (silouette.empty())
			continue;

		// Remove visible faces.
		for (uint32_t j = 0; j < uint32_t(visible.size()); )
		{
			if (visible[j])
			{
				faces.erase(faces.begin() + j);
				visible.erase(visible.begin() + j);
			}
			else
				++j;
		}

		// Add new faces.
		for (std::vector< std::pair< uint32_t, uint32_t > >::iterator j = silouette.begin(); j != silouette.end(); ++j)
		{
			uint32_t idx[] = { j->second, j->first, i };
			faces.push_back(HullFace(idx[0], idx[1], idx[2]));
		}

		// Recalculate adjacency.
		adjacency.resize(0);
		errorCount = calculateAdjacency(faces, adjacency);
		if (errorCount > 0)
			T_DEBUG(errorCount << L" adjanceny error(s)");
	}

	// Clear everything except positions.
	model.clear(Model::CfMaterials | Model::CfVertices | Model::CfPolygons | Model::CfNormals | Model::CfTexCoords | Model::CfJoints);

	model.reservePositions(faces.size() * 3);
	model.reservePolygons(faces.size());

	for (std::vector< HullFace >::iterator i = faces.begin(); i != faces.end(); ++i)
	{
		Vector4 v[] =
		{
			vertices[i->i[0]],
			vertices[i->i[1]],
			vertices[i->i[2]]
		};

		Vector4 e[] =
		{
			v[1] - v[0],
			v[2] - v[0]
		};

		if (cross(e[0], e[1]).length() <= FUZZY_EPSILON)
			continue;

		Polygon polygon;
		for (uint32_t j = 0; j < 3; ++j)
		{
			Vertex vertex;
			vertex.setPosition(i->i[j]);
			polygon.addVertex(model.addVertex(vertex));
		}

		model.addPolygon(polygon);
	}

	if (errorCount > 0)
		log::warning << L"Unable to build complete adjacency of " << errorCount << L" face(s)" << Endl;

	return true;
}

	}
}
