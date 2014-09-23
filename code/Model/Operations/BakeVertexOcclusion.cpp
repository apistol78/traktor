#include "Core/Math/RandomGeometry.h"
#include "Core/Math/SahTree.h"
#include "Model/Model.h"
#include "Model/Operations/BakeVertexOcclusion.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.BakeVertexOcclusion", BakeVertexOcclusion, IModelOperation)

BakeVertexOcclusion::BakeVertexOcclusion(
	int32_t rayCount,
	float raySpread,
	float rayBias
)
:	m_rayCount(rayCount)
,	m_raySpread(raySpread)
,	m_rayBias(rayBias)
{
}

bool BakeVertexOcclusion::apply(Model& model) const
{
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
			w.push(polyVertexPosition);
		}
	}

	// Build acceleration tree.
	SahTree sah;
	sah.build(windings);
	
	SahTree::QueryCache cache;
	for (std::vector< Vertex >::iterator i = vertices.begin(); i != vertices.end(); ++i)
	{
		const Vector4& position = model.getPosition(i->getPosition());
		const Vector4& normal = model.getNormal(i->getNormal());
		
		int32_t occluded = 0;
		for (int32_t j = 0; j < m_rayCount; ++j)
		{
			Vector4 rayDirection = lerp(normal, rnd.nextHemi(normal), Scalar(m_raySpread)).normalized().xyz0();
			Vector4 rayOrigin = (position + normal * Scalar(m_rayBias)).xyz1();
			if (sah.queryAnyIntersection(rayOrigin, rayDirection, 0.0f, cache))
				occluded++;
		}
		
		Vector4 color = Vector4::one();
		
		if (i->getColor() != c_InvalidIndex)
			color = colors[i->getColor()];
			
		color.set(3, Scalar(
			1.0f - occluded / float(m_rayCount)
		));
		
		i->setColor(model.addUniqueColor(color));
	}
	
	model.setVertices(vertices);
	return true;
}

	}
}
