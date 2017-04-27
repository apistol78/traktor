/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstring>
#include <limits>
#include <list>
#include <map>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Core/Math/Half.h"
#include "Core/Math/BspTree.h"
#include "Core/Math/Triangulator.h"
#include "Core/Math/Format.h"
#include "Core/Misc/String.h"
#include "Mesh/Editor/Indoor/IndoorMeshConverter.h"
#include "Mesh/Editor/MeshVertexWriter.h"
#include "Mesh/Indoor/IndoorMeshResource.h"
#include "Model/Model.h"
#include "Render/Mesh/SystemMeshFactory.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshWriter.h"
#include "Render/VertexBuffer.h"
#include "Render/IndexBuffer.h"

namespace traktor
{
	namespace mesh
	{
		namespace
		{

template < typename ValueType >
bool compareEqual(const ValueType& a, const ValueType& b) {
	return a == b;
}

template < >
bool compareEqual(const Vector4& a, const Vector4& b) {
	return (b - a).length() <= FUZZY_EPSILON;
}

template < typename ContainerType >
typename ContainerType::iterator pushUnique(
	ContainerType& ct,
	typename ContainerType::const_reference v,
	bool (*equal)(typename ContainerType::const_reference a, typename ContainerType::const_reference b) = &compareEqual< typename ContainerType::value_type >
)
{
	typename ContainerType::iterator it = ct.begin();
	while (it != ct.end()) { if (equal(*it, v)) break; ++it; }
	return it != ct.end() ? it : ct.insert(ct.end(), v);
}

/* ============================================================================
 * HULL
 */

struct Hull
{
	typedef std::pair< size_t, size_t > edge_t;

	typedef std::vector< size_t > polygon_t;
	typedef std::vector< edge_t > portal_t;

	AlignedVector< Vector4 > points;
	std::vector< polygon_t > polygons;
	std::vector< portal_t > portals;
	BspTree bsp;
};

void createHulls(const model::Model& model, std::map< std::wstring, Hull >& outHulls)
{
	#define HASH(i1, i2) unsigned((i1 > i2) ? (i1 << 16) | i2 : (i2 << 16) | i1)

	/*-----------------------------------------------------------------------*/
	log::info << L"Collecting hulls" << Endl;

	for (std::vector< model::Polygon >::const_iterator i = model.getPolygons().begin(); i != model.getPolygons().end(); ++i)
	{
		std::wstring materialName = model.getMaterial(i->getMaterial()).getName();
		if (!startsWith< std::wstring >(materialName, L"Hull"))
			continue;

		Hull& hull = outHulls[materialName];
		hull.polygons.push_back(Hull::polygon_t());

		for (std::vector< uint32_t >::const_iterator j = i->getVertices().begin(); j != i->getVertices().end(); ++j)
		{
			const model::Vertex& vertex = model.getVertex(*j);
			const Vector4& position = model.getPosition(vertex.getPosition());

			AlignedVector< Vector4 >::iterator it = std::find(hull.points.begin(), hull.points.end(), position);
			if (it == hull.points.end())
				it = hull.points.insert(hull.points.end(), position);

			hull.polygons.back().push_back(uint32_t(std::distance(hull.points.begin(), it)));
		}
	}

	log::info << IncreaseIndent;
	log::info << uint32_t(outHulls.size()) << L" hull(s)" << Endl;
	log::info << DecreaseIndent;

	/*-----------------------------------------------------------------------*/
	log::info << L"Creating hull portals" << Endl;
	log::info << IncreaseIndent;

	for (std::map< std::wstring, Hull >::iterator i = outHulls.begin(); i != outHulls.end(); ++i)
	{
		std::map< unsigned, int > adjc;
		for (std::vector< Hull::polygon_t >::const_iterator j = i->second.polygons.begin(); j != i->second.polygons.end(); ++j)
		{
			for(size_t k = 0, m = j->size() - 1; k < j->size(); m = k++)
				adjc[HASH((*j)[k], (*j)[m])]++;
		}

		std::list< Hull::edge_t > edges;
		for (std::vector< Hull::polygon_t >::const_iterator j = i->second.polygons.begin(); j != i->second.polygons.end(); ++j)
		{
			for(size_t k = 0, m = j->size() - 1; k < j->size(); m = k++)
				if (adjc[HASH((*j)[k], (*j)[m])] == 1)
					edges.push_back(Hull::edge_t((*j)[k], (*j)[m]));
		}

		while (!edges.empty())
		{
			Hull::portal_t portal;

			portal.push_back(edges.front());
			edges.pop_front();

			for (std::list< Hull::edge_t >::iterator j = edges.begin(); j != edges.end(); )
			{
				if (portal.back().second == j->first || portal.back().second == j->second)
				{
					portal.push_back(portal.back().second == j->first ? Hull::edge_t(j->first, j->second) : Hull::edge_t(j->second, j->first));
					edges.erase(j);
					j = edges.begin();
				}
				else
					++j;
			}

			T_ASSERT (portal.front().first == portal.back().second);
			i->second.portals.push_back(portal);
		}

		log::info << L"\"" << i->first << L"\" " << uint32_t(i->second.portals.size()) << L" portal(s)" << Endl;
	}

	log::info << DecreaseIndent;

	/*-----------------------------------------------------------------------*/
	log::info << L"Creating hull BSP trees" << Endl;

	for (std::map< std::wstring, Hull >::iterator i = outHulls.begin(); i != outHulls.end(); ++i)
	{
		AlignedVector< Winding3 > bw;
		for (std::vector< Hull::polygon_t >::const_iterator j = i->second.polygons.begin(); j != i->second.polygons.end(); ++j)
		{
			bw.push_back(Winding3());
			for (Hull::polygon_t::const_iterator k = j->begin(); k != j->end(); ++k)
				bw.back().push(i->second.points[*k]);
		}
		for (std::vector< Hull::portal_t >::const_iterator j = i->second.portals.begin(); j != i->second.portals.end(); ++j)
		{
			bw.push_back(Winding3());
			for (Hull::portal_t::const_iterator k = j->begin(); k != j->end(); ++k)
				bw.back().push(i->second.points[k->first]);
		}
		i->second.bsp.build(bw);
	}
}

/* ============================================================================
 * SECTORS
 */

struct Vertex
{
	Vector4 position;
	Vector4 normal;
	Vector2 texCoord;

	bool operator == (const Vertex& rh) const
	{
		return
			(position - rh.position).length() <= FUZZY_EPSILON &&
			(normal - rh.normal).length() <= FUZZY_EPSILON &&
			(texCoord - rh.texCoord).length() <= FUZZY_EPSILON;
	}
};

struct Polygon
{
	int material;
	std::vector< size_t > indices;
};

struct Sector
{
	std::wstring name;
	AlignedVector< Vertex > vertices;
	std::vector< Polygon > polygons;
	Aabb3 boundingBox;
};

struct Portal
{
	Winding3 winding;
	int32_t sectors[2];
};

struct BspPolygon
{
	AlignedVector< Vertex > vertices;

	Winding3 winding() const
	{
		Winding3 w;
		for (AlignedVector< Vertex >::const_iterator i = vertices.begin(); i != vertices.end(); ++i)
			w.push(i->position);
		return w;
	}

	void split(const Plane& pl, BspPolygon& outFront, BspPolygon& outBack) const
	{
		for (size_t i = 0, j = vertices.size() - 1; i < vertices.size(); j = i++)
		{
			const Vertex& va = vertices[i];
			const Vertex& vb = vertices[j];

			float da = pl.distance(va.position);
			float db = pl.distance(vb.position);

			if ((da < -FUZZY_EPSILON && db > FUZZY_EPSILON) || (da > FUZZY_EPSILON && db < -FUZZY_EPSILON))
			{
				Scalar k(-1.0f);
				pl.segmentIntersection(va.position, vb.position, k);
				T_ASSERT (k >= 0.0f && k <= 1.0f);

				Vertex v;
				v.position = lerp(va.position, vb.position, k);
				v.normal = lerp(va.normal, vb.normal, k);
				v.texCoord = lerp(va.texCoord, vb.texCoord, k);

				outFront.vertices.push_back(v);
				outBack.vertices.push_back(v);
			}

			if (da >= FUZZY_EPSILON)
				outFront.vertices.push_back(va);
			else
				outBack.vertices.push_back(va);
		}
	}

	bool valid() const
	{
		if (vertices.size() < 3)
			return false;

		return true;
	}
};

void createSectors(
	const model::Model& model,
	const std::map< std::wstring, Hull >& hulls,
	AlignedVector< Sector >& outSectors,
	AlignedVector< Portal >& outPortals
)
{
	AlignedVector< Vertex > vertices;
	std::vector< Polygon > polygons;

	/*-----------------------------------------------------------------------*/
	log::info << L"Collecting level polygons" << Endl;

	for (std::vector< model::Polygon >::const_iterator i = model.getPolygons().begin(); i != model.getPolygons().end(); ++i)
	{
		std::wstring materialName = model.getMaterial(i->getMaterial()).getName();
		if (startsWith< std::wstring >(materialName, L"Hull"))
			continue;

		polygons.push_back(Polygon());
		polygons.back().material = i->getMaterial();

		for (std::vector< uint32_t >::const_iterator j = i->getVertices().begin(); j != i->getVertices().end(); ++j)
		{
			const model::Vertex& mv = model.getVertex(*j);

			Vertex v;
			v.position = model.getPosition(mv.getPosition());
			v.normal = mv.getNormal() != model::c_InvalidIndex ? model.getNormal(mv.getNormal()) : Vector4(0.0f, 1.0f, 0.0f, 0.0f);
			v.texCoord = mv.getTexCoord(0) != model::c_InvalidIndex ? model.getTexCoord(mv.getTexCoord(0)) : Vector2(0.0f, 0.0f);

			AlignedVector< Vertex >::iterator it = pushUnique(vertices, v);
			polygons.back().indices.push_back(uint32_t(std::distance(vertices.begin(), it)));
		}
	}

	/*-----------------------------------------------------------------------*/
	log::info << L"Creating sectors from hulls" << Endl;
	log::info << IncreaseIndent;

	for (std::map< std::wstring, Hull >::const_iterator i = hulls.begin(); i != hulls.end(); ++i)
	{
		outSectors.push_back(Sector());
		outSectors.back().name = i->first;
		for (std::vector< Polygon >::const_iterator j = polygons.begin(); j != polygons.end(); ++j)
		{
			BspPolygon input;
			for (std::vector< size_t >::const_iterator k = j->indices.begin(); k != j->indices.end(); ++k)
				input.vertices.push_back(vertices[*k]);

			AlignedVector< BspPolygon > clipped;
			i->second.bsp.clip< BspPolygon >(input, BspTree::CmFront, clipped);
			if (clipped.empty())
				continue;

			for (AlignedVector< BspPolygon >::const_iterator k = clipped.begin(); k != clipped.end(); ++k)
			{
				outSectors.back().polygons.push_back(Polygon());
				outSectors.back().polygons.back().material = j->material;
				for (AlignedVector< Vertex >::const_iterator m = k->vertices.begin(); m != k->vertices.end(); ++m)
				{
					AlignedVector< Vertex >::iterator it = pushUnique(outSectors.back().vertices, *m);
					outSectors.back().polygons.back().indices.push_back(uint32_t(std::distance(outSectors.back().vertices.begin(), it)));
				}
			}
		}

		log::info << L"\"" << i->first << L"\" " << uint32_t(outSectors.back().polygons.size()) << L" polygon(s)" << Endl;
	}

	log::info << DecreaseIndent;

	/*-----------------------------------------------------------------------*/
	log::info << L"Creating portals" << Endl;
	log::info << IncreaseIndent;

	typedef std::pair< size_t, size_t > edge_t;
	typedef std::vector< size_t > portal_t;

	AlignedVector< Vector4 > points;
	std::list< std::pair< size_t, portal_t > > portals;

	for (AlignedVector< Sector >::iterator i = outSectors.begin(); i != outSectors.end(); ++i)
	{
		const Hull& hull = hulls.find(i->name)->second;
		for (std::vector< Hull::portal_t >::const_iterator j = hull.portals.begin(); j != hull.portals.end(); ++j)
		{
			Winding3 wp;
			for (Hull::portal_t::const_iterator k = j->begin(); k != j->end(); ++k)
				wp.push(hull.points[k->first]);

			Plane portalPlane;
			if (!wp.getPlane(portalPlane))
				continue;

			// Collect points from level which is coplanar with hull portal.
			AlignedVector< Vector4 > coplanarPoints;
			for (std::vector< Polygon >::const_iterator k = i->polygons.begin(); k != i->polygons.end(); ++k)
			{
				for (std::vector< size_t >::const_iterator it = k->indices.begin(); it != k->indices.end(); ++it)
				{
					const Vector4& p = i->vertices[*it].position;
					if (abs(portalPlane.distance(p)) > FUZZY_EPSILON)
						continue;

					coplanarPoints.push_back(p);
				}
			}
			if (coplanarPoints.empty())
				continue;

			// Create a portal from all the coplanar points, could potentially extract the "2d" hull but for
			// now we just use the projected rectangle.

			const Vector4 up(0.0f, 1.0f, 0.0f, 0.0f);

			Vector4 px =  cross(portalPlane.normal(), up).normalized();
			Vector4 py = -cross(px, portalPlane.normal()).normalized();

			Vector2 mn(std::numeric_limits< float >::max(), std::numeric_limits< float >::max());
			Vector2 mx = -mn;

			for (AlignedVector< Vector4 >::const_iterator k = coplanarPoints.begin(); k != coplanarPoints.end(); ++k)
			{
				mn.x = min< float >(mn.x, dot3(*k, px));
				mn.y = min< float >(mn.y, dot3(*k, py));
				mx.x = max< float >(mx.x, dot3(*k, px));
				mx.y = max< float >(mx.y, dot3(*k, py));
			}

			Vector4 base = (portalPlane.normal() * portalPlane.distance()).xyz1();

			Vector4 r[] =
			{
				base + px * Scalar(mn.x) + py * Scalar(mn.y),
				base + px * Scalar(mx.x) + py * Scalar(mn.y),
				base + px * Scalar(mx.x) + py * Scalar(mx.y),
				base + px * Scalar(mn.x) + py * Scalar(mx.y)
			};

			portals.push_back(std::make_pair(
				std::distance(outSectors.begin(), i),
				portal_t()
			));

			for (uint32_t k = 0; k < sizeof_array(r); ++k)
			{
				AlignedVector< Vector4 >::iterator it = pushUnique(points, r[k]);
				portals.back().second.push_back(uint32_t(std::distance(points.begin(), it)));
				log::info << k << L". " << r[k] << Endl;
			}
		}
	}

	log::info << uint32_t(portals.size()) << L" portal(s)" << Endl;
	log::info << DecreaseIndent;

	/*-----------------------------------------------------------------------*/
	log::info << L"Linking portals" << Endl;
	log::info << IncreaseIndent;

	while (!portals.empty())
	{
		std::pair< size_t, portal_t > portal = portals.back();
		portals.pop_back();

		portal_t p1 = portal.second; std::sort(p1.begin(), p1.end());
		std::list< std::pair< size_t, portal_t > >::iterator it = portals.begin();
		while (it != portals.end())
		{
			portal_t p2 = it->second; std::sort(p2.begin(), p2.end());
			if (p1 == p2)
				break;
			++it;
		}

		outPortals.push_back(Portal());
		for (portal_t::const_iterator i = portal.second.begin(); i != portal.second.end(); ++i)
		{
			AlignedVector< Vector4 >::iterator it = points.begin(); std::advance(it, *i);
			outPortals.back().winding.push(*it);
		}

		outPortals.back().sectors[0] = int(portal.first);
		if (it != portals.end())
		{
			outPortals.back().sectors[1] = int(it->first);
			portals.erase(it);
		}
		else
			outPortals.back().sectors[1] = -1;

		log::info << outPortals.back().sectors[0] << L" <-> " << outPortals.back().sectors[1] << Endl;
	}

	log::info << uint32_t(outPortals.size()) << L" linked portal(s)" << Endl;
	log::info << DecreaseIndent;

	/*-----------------------------------------------------------------------*/
	log::info << L"Triangulating sector polygons" << Endl;
	log::info << IncreaseIndent;

	for (AlignedVector< Sector >::iterator i = outSectors.begin(); i != outSectors.end(); ++i)
	{
		std::vector< Polygon > triangles;
		for (std::vector< Polygon >::const_iterator j = i->polygons.begin(); j != i->polygons.end(); ++j)
		{
			Winding3 winding;
			for (std::vector< size_t >::const_iterator k = j->indices.begin(); k != j->indices.end(); ++k)
				winding.push(i->vertices[*k].position);

			Plane windingPlane;
			winding.getPlane(windingPlane);

			std::vector< Triangulator::Triangle > triangulation;
			Triangulator().freeze(
				winding.getPoints(),
				windingPlane.normal(),
				triangulation
			);

			for (std::vector< Triangulator::Triangle >::const_iterator k = triangulation.begin(); k != triangulation.end(); ++k)
			{
				triangles.push_back(Polygon());
				triangles.back().material = j->material;
				triangles.back().indices.push_back(j->indices[k->indices[0]]);
				triangles.back().indices.push_back(j->indices[k->indices[1]]);
				triangles.back().indices.push_back(j->indices[k->indices[2]]);
			}
		}
		i->polygons = triangles;
		log::info << L"\"" << i->name << L"\" " << uint32_t(triangles.size()) << L" triangle(s)" << Endl;
	}

	log::info << DecreaseIndent;

	log::info << L"Calculating sector bounding boxes" << Endl;
	log::info << IncreaseIndent;

	for (AlignedVector< Sector >::iterator i = outSectors.begin(); i != outSectors.end(); ++i)
	{
		for (AlignedVector< Vertex >::iterator j = i->vertices.begin(); j != i->vertices.end(); ++j)
			i->boundingBox.contain(j->position);
		log::info << L"\"" << i->name << L"\" min " << i->boundingBox.mn << L", max " << i->boundingBox.mx << Endl;
	}

	log::info << DecreaseIndent;
}

			}

/* ============================================================================
 * CONVERTER
 */

Ref< IMeshResource > IndoorMeshConverter::createResource() const
{
	return new IndoorMeshResource();
}

bool IndoorMeshConverter::convert(
	const MeshAsset* meshAsset,
	const RefArray< model::Model >& models,
	const Guid& materialGuid,
	const std::map< std::wstring, std::list< MeshMaterialTechnique > >& materialTechniqueMap,
	const std::vector< render::VertexElement >& vertexElements,
	int32_t maxInstanceCount,
	IMeshResource* meshResource,
	IStream* meshResourceStream
) const
{
	const model::Model& model = *models[0];
	std::map< std::wstring, Hull > hulls;
	AlignedVector< Sector > sectors;
	AlignedVector< Portal > portals;

	/*-----------------------------------------------------------------------*/
	log::info << L"Creating portalization" << Endl;

	createHulls(model, hulls);
	createSectors(model, hulls, sectors, portals);

	/*-----------------------------------------------------------------------*/
	log::info << L"Creating indoor mesh" << Endl;

	uint32_t vertexCount = 0;
	uint32_t triangleCount = 0;
	for (AlignedVector< Sector >::const_iterator i = sectors.begin(); i != sectors.end(); ++i)
	{
		vertexCount += uint32_t(i->vertices.size());
		triangleCount += uint32_t(i->polygons.size());
	}

	log::info << IncreaseIndent;
	log::info << vertexCount << L" vertices" << Endl;
	log::info << triangleCount << L" triangles" << Endl;
	log::info << DecreaseIndent;

	uint32_t vertexSize = render::getVertexSize(vertexElements);
	T_ASSERT (vertexSize > 0);

	uint32_t vertexBufferSize = vertexCount * vertexSize;
	uint32_t indexBufferSize = triangleCount * 3 * sizeof(uint16_t);

	Ref< render::Mesh > mesh = render::SystemMeshFactory().createMesh(
		vertexElements,
		vertexBufferSize,
		render::ItUInt16,
		indexBufferSize
	);

	/*-----------------------------------------------------------------------*/
	log::info << L"Creating vertex buffer" << Endl;

	uint8_t* vertex = static_cast< uint8_t* >(mesh->getVertexBuffer()->lock());
	for (AlignedVector< Sector >::const_iterator i = sectors.begin(); i != sectors.end(); ++i)
	{
		for (AlignedVector< Vertex >::const_iterator j = i->vertices.begin(); j != i->vertices.end(); ++j)
		{
			std::memset(vertex, 0, vertexSize);

			writeVertexData(vertexElements, vertex, render::DuPosition, 0, j->position);
			writeVertexData(vertexElements, vertex, render::DuNormal, 0, j->normal);
			writeVertexData(vertexElements, vertex, render::DuCustom, 0, j->texCoord);

			vertex += vertexSize;
		}
	}
	mesh->getVertexBuffer()->unlock();

	/*-----------------------------------------------------------------------*/
	log::info << L"Creating index buffer" << Endl;

	AlignedVector< IndoorMeshResource::Sector > assetSectors;
	AlignedVector< IndoorMeshResource::Portal > assetPortals;

	size_t indexOffset = 0;
	size_t vertexBase = 0;
	uint16_t* index = static_cast< uint16_t* >(mesh->getIndexBuffer()->lock());

	for (AlignedVector< Sector >::const_iterator i = sectors.begin(); i != sectors.end(); ++i)
	{
		assetSectors.push_back(IndoorMeshResource::Sector());
		assetSectors.back().min = i->boundingBox.mn;
		assetSectors.back().max = i->boundingBox.mx;

		for (uint32_t material = 0; material < uint32_t(model.getMaterials().size()); ++material)
		{
			std::map< std::wstring, std::list< MeshMaterialTechnique > >::const_iterator materialIt = materialTechniqueMap.find(model.getMaterial(material).getName());
			if (materialIt == materialTechniqueMap.end())
				continue;

			size_t minIndex = std::numeric_limits< size_t >::max();
			size_t maxIndex = 0;
			size_t indexCount = 0;

			for (std::vector< Polygon >::const_iterator j = i->polygons.begin(); j != i->polygons.end(); ++j)
			{
				if (j->material != material)
					continue;

				T_ASSERT (j->indices.size() == 3);
				for (int k = 0; k < 3; ++k)
				{
					*index++ = (uint16_t)(vertexBase + j->indices[k]);
					minIndex = std::min(j->indices[k], minIndex);
					maxIndex = std::max(j->indices[k], maxIndex);
					indexCount++;
				}
			}

			if (!indexCount)
				continue;

			for (std::list< MeshMaterialTechnique >::const_iterator j = materialIt->second.begin(); j != materialIt->second.end(); ++j)
			{
				IndoorMeshResource::Part part;
				part.shaderTechnique = j->shaderTechnique;
				//part.primitives.setIndexed(
				//	render::PtTriangles,
				//	int(indexOffset),
				//	int(indexCount / 3),
				//	int(minIndex),
				//	int(maxIndex)
				//);
				assetSectors.back().parts[j->worldTechnique].push_back(part);
			}

			indexOffset += indexCount;
		}

		vertexBase += i->vertices.size();
	}

	mesh->getIndexBuffer()->unlock();

	for (AlignedVector< Portal >::const_iterator i = portals.begin(); i != portals.end(); ++i)
	{
		assetPortals.push_back(IndoorMeshResource::Portal());
		assetPortals.back().pts = i->winding.getPoints();
		assetPortals.back().sectorA = i->sectors[0];
		assetPortals.back().sectorB = i->sectors[1];
	}

	/*-----------------------------------------------------------------------*/
	log::info << L"Creating asset" << Endl;

	mesh->setBoundingBox(model.getBoundingBox());

	if (!render::MeshWriter().write(meshResourceStream, mesh))
		return false;

	checked_type_cast< IndoorMeshResource* >(meshResource)->m_shader = resource::Id< render::Shader >(materialGuid);
	checked_type_cast< IndoorMeshResource* >(meshResource)->m_sectors = assetSectors;
	checked_type_cast< IndoorMeshResource* >(meshResource)->m_portals = assetPortals;

	/*-----------------------------------------------------------------------*/
	log::info << L"Finished" << Endl;

	return true;
}

	}
}
