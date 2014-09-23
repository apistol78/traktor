#include "Core/Log/Log.h"
#include "Core/Math/Winding3.h"
#include "Model/Model.h"
#include "Model/Operations/CullDistantFaces.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.CullDistantFaces", CullDistantFaces, IModelOperation)

CullDistantFaces::CullDistantFaces(const Aabb3& viewerRegion)
:	m_viewerRegion(viewerRegion)
{
}

bool CullDistantFaces::apply(Model& model) const
{
	Vector4 viewerCorners[8];
	m_viewerRegion.getExtents(viewerCorners);

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
			winding.push(position);
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
	return true;
}

	}
}
