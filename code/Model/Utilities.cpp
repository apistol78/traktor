#include <algorithm>
#include <numeric>
#include "Core/Containers/AlignedVector.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Triangulator.h"
#include "Core/Math/Plane.h"
#include "Core/Math/RandomGeometry.h"
#include "Core/Math/SahTree.h"
#include "Core/Math/Winding3.h"
#include "Model/ContainerHelpers.h"
#include "Model/Model.h"
#include "Model/TriangleOrderForsyth.h"
#include "Model/Utilities.h"

namespace traktor
{
	namespace model
	{
		namespace
		{

struct TangentBase
{
	Vector4 normal;
	Vector4 tangent;
	Vector4 binormal;

	TangentBase()
	:	normal(0.0f, 0.0f, 0.0f, 0.0f)
	,	tangent(0.0f, 0.0f, 0.0f, 0.0f)
	,	binormal(0.0f, 0.0f, 0.0f, 0.0f)
	{
	}
};

		}

Aabb3 calculateModelBoundingBox(const Model& model)
{
	Aabb3 boundingBox;
	const AlignedVector< Vector4 >& positions = model.getPositions();
	for (AlignedVector< Vector4 >::const_iterator i = positions.begin(); i != positions.end(); ++i)
		boundingBox.contain(*i);
	return boundingBox;
}

namespace
{

	bool findBaseIndex(const Model& mdl, const Polygon& polygon, uint32_t& outBaseIndex)
	{
		outBaseIndex = c_InvalidIndex;

		const std::vector< uint32_t >& vertices = polygon.getVertices();
		for (uint32_t i = 0; i < uint32_t(vertices.size()); ++i)
		{
			const Vertex* v[] =
			{
				&mdl.getVertex(vertices[i]),
				&mdl.getVertex(vertices[(i + 1) % vertices.size()]),
				&mdl.getVertex(vertices[(i + 2) % vertices.size()])
			};

			Vector4 p[] =
			{
				mdl.getPosition(v[0]->getPosition()),
				mdl.getPosition(v[1]->getPosition()),
				mdl.getPosition(v[2]->getPosition())
			};

			Vector4 ep[] = { p[2] - p[0], p[1] - p[0] };
			if (ep[0].length() > FUZZY_EPSILON && ep[1].length() > FUZZY_EPSILON && cross(ep[0], ep[1]).length() > FUZZY_EPSILON)
			{
				outBaseIndex = i;
				return true;
			}
		}

		return false;
	}

}

void calculateModelTangents(Model& mdl, bool binormals)
{
	const std::vector< Polygon >& polygons = mdl.getPolygons();
	AlignedVector< TangentBase > polygonTangentBases;
	AlignedVector< TangentBase > vertexTangentBases;
	uint32_t degenerated = 0;
	uint32_t invalid = 0;

	// Calculate tangent base for each polygon.
	polygonTangentBases.resize(polygons.size());
	for (uint32_t i = 0; i < uint32_t(polygons.size()); ++i)
	{
		const Polygon& polygon = polygons[i];
		uint32_t baseIndex;

		if (!findBaseIndex(mdl, polygon, baseIndex))
		{
			++degenerated;
			continue;
		}

		const std::vector< uint32_t >& vertices = polygon.getVertices();
		T_ASSERT (vertices.size() == 3);

		const Vertex* v[] =
		{
			&mdl.getVertex(vertices[baseIndex]),
			&mdl.getVertex(vertices[(baseIndex + 1) % vertices.size()]),
			&mdl.getVertex(vertices[(baseIndex + 2) % vertices.size()])
		};

		Vector4 p[] =
		{
			mdl.getPosition(v[0]->getPosition()),
			mdl.getPosition(v[1]->getPosition()),
			mdl.getPosition(v[2]->getPosition())
		};

		Vector4 ep[] = { p[2] - p[0], p[1] - p[0] };
		T_ASSERT (ep[0].length() > FUZZY_EPSILON);
		T_ASSERT (ep[1].length() > FUZZY_EPSILON);

		ep[0] = ep[0].normalized();
		ep[1] = ep[1].normalized();

		TangentBase& tb = polygonTangentBases[i];
		tb.normal = cross(ep[0], ep[1]);
		tb.tangent = cross(Vector4(0.0f, 1.0f, 0.0f, 0.0f), tb.normal);
		tb.binormal = cross(tb.tangent, tb.normal);

		T_ASSERT (tb.normal.length() > FUZZY_EPSILON);
		tb.normal = tb.normal.normalized();
		
		if (v[0]->getTexCoord(0) != c_InvalidIndex && v[1]->getTexCoord(0) != c_InvalidIndex && v[2]->getTexCoord(0) != c_InvalidIndex)
		{
			Vector2 tc[] =
			{
				mdl.getTexCoord(v[0]->getTexCoord(0)),
				mdl.getTexCoord(v[1]->getTexCoord(0)),
				mdl.getTexCoord(v[2]->getTexCoord(0))
			};

			Vector2 etc[] = { tc[2] - tc[0], tc[1] - tc[0] };

			float denom = etc[0].x * etc[1].y - etc[1].x * etc[0].y;
			Scalar r = Scalar(denom != 0.0f ? 1.0f / denom : 0.0f);

			tb.tangent = ((Scalar(etc[0].y) * ep[1] - Scalar(etc[1].y) * ep[0]) * r).xyz0();
			tb.binormal = ((Scalar(etc[1].x) * ep[0] - Scalar(etc[0].x) * ep[1]) * r).xyz0();

			bool tangentValid = tb.tangent.length() > FUZZY_EPSILON;
			bool binormalValid = tb.binormal.length() > FUZZY_EPSILON;

			if (tangentValid || binormalValid)
			{
				if (!tangentValid)
					tb.tangent = cross(tb.binormal, tb.normal);
				if (!binormalValid)
					tb.binormal = cross(tb.tangent, tb.normal);
				
				tb.tangent = tb.tangent.normalized();
				tb.binormal = tb.binormal.normalized();

				Vector4 normal = cross(tb.tangent, tb.binormal);
				if (normal.length() >= FUZZY_EPSILON)
				{
					if (dot3(normal.normalized(), tb.normal) < 0.0f)
						tb.tangent = -tb.tangent;
				}
				else
					++invalid;
			}
			else
				++invalid;
		}
	}

	if (degenerated)
		log::warning << L"Degenerate " << degenerated << L" polygon(s) found in model" << Endl;

	if (invalid)
		log::warning << L"Invalid tangent space vectors; " << invalid << L" invalid texture coordinate(s)" << Endl;

	// Normalize polygon tangent bases.
	for (AlignedVector< TangentBase >::iterator i = polygonTangentBases.begin(); i != polygonTangentBases.end(); ++i)
	{
		if (i->normal.length() > FUZZY_EPSILON)
			i->normal = i->normal.normalized();
		if (i->tangent.length() > FUZZY_EPSILON)
			i->tangent = i->tangent.normalized();
		if (i->binormal.length() > FUZZY_EPSILON)
			i->binormal = i->binormal.normalized();
	}

	// Build new vertex normals.
	vertexTangentBases.resize(mdl.getVertexCount());
	for (uint32_t i = 0; i < uint32_t(polygons.size()); ++i)
	{
		const Polygon& polygon = polygons[i];

		const std::vector< uint32_t >& vertices = polygon.getVertices();
		for (std::vector< uint32_t >::const_iterator j = vertices.begin(); j != vertices.end(); ++j)
		{
			vertexTangentBases[*j].normal += polygonTangentBases[i].normal;
			vertexTangentBases[*j].tangent += polygonTangentBases[i].tangent;
			vertexTangentBases[*j].binormal += polygonTangentBases[i].binormal;
		}
	}

	// Normalize vertex tangent bases.
	for (AlignedVector< TangentBase >::iterator i = vertexTangentBases.begin(); i != vertexTangentBases.end(); ++i)
	{
		if (i->normal.length() > FUZZY_EPSILON)
			i->normal = i->normal.normalized();
		if (i->tangent.length() > FUZZY_EPSILON)
			i->tangent = i->tangent.normalized();
		if (i->binormal.length() > FUZZY_EPSILON)
			i->binormal = i->binormal.normalized();
	}

	// Update vertices.
	for (uint32_t i = 0; i < mdl.getVertexCount(); ++i)
	{
		const TangentBase& tb = vertexTangentBases[i];

		Vertex vertex = mdl.getVertex(i);
		if (vertex.getNormal() == c_InvalidIndex)
			vertex.setNormal(mdl.addUniqueNormal(tb.normal));
		if (vertex.getTangent() == c_InvalidIndex)
			vertex.setTangent(mdl.addUniqueNormal(tb.tangent));
		if (vertex.getBinormal() == c_InvalidIndex)
			vertex.setBinormal(mdl.addUniqueNormal(tb.binormal));

		mdl.setVertex(i, vertex);
	}
}

void triangulateModel(Model& model)
{
	std::vector< Polygon > triangulatedPolygons;
	std::vector< Triangulator::Triangle > triangles;

	const std::vector< Polygon >& polygons = model.getPolygons();
	triangulatedPolygons.reserve(polygons.size());

	for (std::vector< Polygon >::const_iterator i = polygons.begin(); i != polygons.end(); ++i)
	{
		const std::vector< uint32_t >& vertices = i->getVertices();
		if (vertices.size() > 3)
		{
			Winding3 polygonWinding;

			polygonWinding.points.resize(vertices.size());
			for (size_t j = 0; j < vertices.size(); ++j)
				polygonWinding.points[j] = model.getPosition(model.getVertex(vertices[j]).getPosition());

			Vector4 polygonNormal;
			if (i->getNormal() != c_InvalidIndex)
				polygonNormal = model.getNormal(i->getNormal());
			else
			{
				// No normal associated with polygon; try to determine from winding.
				Plane polygonPlane;
				if (!polygonWinding.getPlane(polygonPlane))
					continue;

				polygonNormal = polygonPlane.normal();
			}

			triangles.resize(0);
			Triangulator().freeze(
				polygonWinding.points,
				polygonNormal,
				triangles
			);

			for (std::vector< Triangulator::Triangle >::const_iterator j = triangles.begin(); j != triangles.end(); ++j)
			{
				Polygon triangulatedPolygon(
					i->getMaterial(),
					vertices[j->indices[0]],
					vertices[j->indices[1]],
					vertices[j->indices[2]]
				);
				triangulatedPolygons.push_back(triangulatedPolygon);
			}
		}
		else if (vertices.size() == 3)
			triangulatedPolygons.push_back(*i);
	}

	model.setPolygons(triangulatedPolygons);
}

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

	void calculateAdjacency(const std::vector< HullFace >& faces, std::vector< HullFaceAdjacency >& outAdjacency)
	{
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

						if (a1 == na2 && a2 == na1)
						{
							adj.n[j] = k;
							break;
						}
					}
				}
			}

			if (adj.n[0] == ~0U || adj.n[1] == ~0U || adj.n[2] == ~0U)
				log::warning << L"Unable to build complete adjacency of face " << i << Endl;

			outAdjacency.push_back(adj);
		}
	}

}

void calculateConvexHull(Model& model)
{
	AlignedVector< Vector4 > vertices = model.getPositions();

	uint32_t t = ~0U;
	for (int i = 0; i < 2; ++i)
	{
		Plane pl(vertices[0], vertices[1], vertices[2]);
		for (uint32_t i = 3; i < uint32_t(vertices.size()); ++i)
		{
			if (pl.distance(vertices[i]) < -FUZZY_EPSILON)
			{
				t = i;
				break;
			}
		}
		if (t == ~0U)
			std::swap(vertices[0], vertices[1]);
		else
			break;
	}
	T_ASSERT (t != ~0U);

	std::vector< HullFace > faces;
	faces.reserve(32);
	faces.push_back(HullFace(0, 1, 2));
	faces.push_back(HullFace(1, 0, t));
	faces.push_back(HullFace(2, 1, t));
	faces.push_back(HullFace(0, 2, t));

	std::vector< HullFaceAdjacency > adjacency;
	calculateAdjacency(faces, adjacency);

	for (uint32_t i = 0; i < uint32_t(vertices.size()); ++i)
	{
		std::vector< bool > visible(faces.size());
		for (uint32_t j = 0; j < uint32_t(faces.size()); ++j)
		{
			Plane pl(
				vertices[faces[j].i[0]],
				vertices[faces[j].i[1]],
				vertices[faces[j].i[2]]
			);
			visible[j] = bool(pl.distance(vertices[i]) >= -FUZZY_EPSILON);
		}

		std::vector< std::pair< uint32_t, uint32_t > > silouette;
		for (uint32_t j = 0; j < uint32_t(faces.size()); ++j)
		{
			for (uint32_t k = 0; k < 3; ++k)
			{
				uint32_t n = adjacency[j].n[k];
				if (n != ~0UL && !visible[j] && visible[n])
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
			int idx[] = { j->second, j->first, i };
			faces.push_back(HullFace(idx[0], idx[1], idx[2]));
		}
		
		// Recalculate adjacency.
		adjacency.resize(0);
		calculateAdjacency(faces, adjacency);
	}

	// Clear everything except positions.
	model.clear(Model::CfMaterials | Model::CfVertices | Model::CfPolygons | Model::CfNormals | Model::CfTexCoords | Model::CfBones);

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
}

namespace
{

	struct SortPolygonDistance
	{
		bool operator () (const Polygon& a, const Polygon& b) const
		{
			const std::vector< uint32_t >& va = a.getVertices();
			const std::vector< uint32_t >& vb = b.getVertices();

			uint32_t ia = std::accumulate(va.begin(), va.end(), 0);
			uint32_t ib = std::accumulate(vb.begin(), vb.end(), 0);

			return ia < ib;
		}
	};

}

void sortPolygonsCacheCoherent(Model& model)
{
	std::vector< Polygon > polygons = model.getPolygons();
	if (polygons.size() <= 2)
		return;

	std::vector< Polygon > newPolygons;
	newPolygons.reserve(polygons.size());

	uint32_t materialCount = model.getMaterials().size();
	for (uint32_t material = 0; material < materialCount; ++material)
	{
		std::vector< uint32_t > indices;
		for (std::vector< Polygon >::const_iterator i = polygons.begin(); i != polygons.end(); ++i)
		{
			if (i->getMaterial() == material)
				indices.insert(indices.end(), i->getVertices().begin(), i->getVertices().end());
		}

		if (indices.empty())
			continue;

		std::vector< uint32_t >::const_iterator it = std::max_element(indices.begin(), indices.end());
		uint32_t vertexCount = *it + 1;

		std::vector< uint32_t > newIndices(indices.size());
		optimizeFaces(
			indices,
			vertexCount,
			newIndices,
			32
		);

		for (uint32_t i = 0; i < newIndices.size(); i += 3)
		{
			newPolygons.push_back(Polygon(
				material,
				newIndices[i + 0],
				newIndices[i + 1],
				newIndices[i + 2]
			));
		}
	}

	model.setPolygons(newPolygons);
}

void cleanDuplicates(Model& model)
{
	std::vector< Vertex > vertices = model.getVertices();
	std::vector< Polygon > polygons = model.getPolygons();

	for (std::vector< Vertex >::iterator i = vertices.begin(); i != vertices.end() - 1; )
	{
		uint32_t duplicateIndex = c_InvalidIndex;
		for (std::vector< Vertex >::iterator j = i + 1; j != vertices.end(); ++j)
		{
			if (*i == *j)
			{
				duplicateIndex = uint32_t(std::distance(vertices.begin(), j));
				break;
			}
		}
		if (duplicateIndex != c_InvalidIndex)
		{
			uint32_t originalIndex = uint32_t(std::distance(vertices.begin(), i));

			for (std::vector< Polygon >::iterator j = polygons.begin(); j != polygons.end(); ++j)
			{
				for (uint32_t k = 0; k < j->getVertexCount(); ++k)
				{
					if (j->getVertex(k) == originalIndex)
						j->setVertex(k, duplicateIndex);
				}
			}

			i = vertices.erase(i);
		}
		else
			i++;
	}

	for (std::vector< Polygon >::iterator i = polygons.begin(); i != polygons.end() - 1; )
	{
		bool duplicate = false;
		for (std::vector< Polygon >::iterator j = i + 1; j != polygons.end(); ++j)
		{
			if (*i == *j)
			{
				duplicate = true;
				break;
			}
		}
		if (duplicate)
			i = polygons.erase(i);
		else
			i++;
	}

	model.setVertices(vertices);
	model.setPolygons(polygons);
}

void flattenDoubleSided(Model& model)
{
	std::vector< Polygon > polygons = model.getPolygons();
	std::vector< Polygon > flatten; flatten.reserve(polygons.size());

	for (std::vector< Polygon >::const_iterator i = polygons.begin(); i != polygons.end(); ++i)
	{
		uint32_t materialId = i->getMaterial();
		const Material& material = model.getMaterial(materialId);
		if (!material.isDoubleSided())
			continue;

		Polygon flat = *i;
		for (uint32_t j = 0; j < i->getVertexCount(); ++j)
		{
			uint32_t vtx = i->getVertex(j);
			flat.setVertex(i->getVertexCount() - j - 1, vtx);
		}
		flatten.push_back(flat);
	}

	flatten.insert(flatten.end(), polygons.begin(), polygons.end());
	model.setPolygons(flatten);

	std::vector< Material > materials = model.getMaterials();
	for (std::vector< Material >::iterator i = materials.begin(); i != materials.end(); ++i)
		i->setDoubleSided(false);
	model.setMaterials(materials);
}

void bakeVertexOcclusion(Model& model)
{
	const uint32_t c_occlusionRayCount = 64;
	const Scalar c_occlusionRaySpread(0.75f);
	const Scalar c_occlusionRayBias(0.1f);

	RandomGeometry rnd;

	const std::vector< Polygon >& polygons = model.getPolygons();
	std::vector< Vertex > vertices = model.getVertices();
	
	AlignedVector< Vector4 > colors = model.getColors();
	model.clear(Model::CfColors);
	
	AlignedVector< Winding3 > windings(polygons.size());
	for (uint32_t i = 0; i < polygons.size(); ++i)
	{
		Winding3& w = windings[i];
		const std::vector< uint32_t >& vertexIndices = polygons[i].getVertices();
		for (std::vector< uint32_t >::const_iterator j = vertexIndices.begin(); j != vertexIndices.end(); ++j)
		{
			const Vertex& polyVertex = model.getVertex(*j);
			const Vector4& polyVertexPosition = model.getPosition(polyVertex.getPosition());
			w.points.push_back(polyVertexPosition);
		}
	}

	// Build acceleration tree.
	SahTree sah;
	sah.build(windings);
	
	for (std::vector< Vertex >::iterator i = vertices.begin(); i != vertices.end(); ++i)
	{
		const Vector4& position = model.getPosition(i->getPosition());
		const Vector4& normal = model.getNormal(i->getNormal());
		
		uint32_t occluded = 0;
		for (uint32_t j = 0; j < c_occlusionRayCount; ++j)
		{
			Vector4 rayDirection = lerp(normal, rnd.nextHemi(normal), c_occlusionRaySpread).normalized().xyz0();
			Vector4 rayOrigin = (position + normal * c_occlusionRayBias).xyz1();
			if (sah.queryAnyIntersection(rayOrigin, rayDirection, 0.0f))
				occluded++;
		}
		
		Vector4 color = Vector4::one();
		
		if (i->getColor() != c_InvalidIndex)
			color = colors[i->getColor()];
			
		color.set(3, Scalar(
			1.0f - occluded / float(c_occlusionRayCount)
		));
		
		i->setColor(model.addUniqueColor(color));
	}
	
	model.setVertices(vertices);
}

void cullDistantFaces(Model& model)
{
	Aabb3 viewerRegion(
		Vector4(-40.0f, -40.0f, -40.0f),
		Vector4( 40.0f,  40.0f,  40.0f)
	);

	Vector4 viewerCorners[8];
	viewerRegion.getExtents(viewerCorners);

	std::vector< Polygon > polygons = model.getPolygons();
	uint32_t originalCount = polygons.size();

	for (uint32_t i = 0; i < polygons.size(); )
	{
		uint32_t vertexCount = polygons[i].getVertexCount();
		if (vertexCount < 3)
		{
			++i;
			continue;
		}

		Winding3 winding;
		for (uint32_t j = 0; j < vertexCount; ++j)
		{
			const Vertex& vertex = model.getVertex(polygons[i].getVertex(j));
			const Vector4& position = model.getPosition(vertex.getPosition());
			winding.points.push_back(position);
		}

		Plane plane;
		if (!winding.getPlane(plane))
		{
			++i;
			continue;
		}

		Vector4 normal = plane.normal();

		bool visible = false;
		for (uint32_t j = 0; j < 8 && !visible; ++j)
		{
			for (uint32_t k = 0; k < vertexCount && !visible; ++k)
			{
				const Vertex& vertex = model.getVertex(polygons[i].getVertex(k));
				const Vector4& position = model.getPosition(vertex.getPosition());
				if (dot3(viewerCorners[j] - position, normal) > 0.0f)
					visible = true;
			}
		}

		if (!visible)
			polygons.erase(polygons.begin() + i);
		else
			++i;
	}
	model.setPolygons(polygons);

	log::info << L"Culled " << uint32_t(originalCount - polygons.size()) << L" polygon(s)" << Endl;
}

	}
}
