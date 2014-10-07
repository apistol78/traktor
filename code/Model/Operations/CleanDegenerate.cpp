#include "Core/Math/Const.h"
#include "Model/Model.h"
#include "Model/Operations/CleanDegenerate.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.CleanDegenerate", CleanDegenerate, IModelOperation)

bool CleanDegenerate::apply(Model& model) const
{
	std::vector< Polygon >& polygons = model.getPolygons();
	for (uint32_t i = 0; i < polygons.size(); )
	{
		Polygon& polygon = polygons[i];

		std::vector< uint32_t >& vertices = polygon.getVertices();
		if (vertices.size() > 1)
		{
			for (uint32_t j = 0; j < vertices.size(); )
			{
				uint32_t p0 = model.getVertex(vertices[j]).getPosition();
				uint32_t p1 = model.getVertex(vertices[(j + 1) % vertices.size()]).getPosition();
				if (p0 == p1)
					vertices.erase(vertices.begin() + j);
				else
					++j;
			}
		}

		if (vertices.size() <= 2)
			polygons.erase(polygons.begin() + i);
		else
			++i;
	}
	return true;
}

	}
}
