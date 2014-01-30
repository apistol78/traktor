#include "Core/Math/Const.h"
#include "Model/Model.h"
#include "Model/Operations/CleanDuplicates.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.CleanDuplicates", CleanDuplicates, IModelOperation)

CleanDuplicates::CleanDuplicates(float positionDistance)
:	m_positionDistance(positionDistance)
{
}

bool CleanDuplicates::apply(Model& model) const
{
	Model cleaned;
	uint32_t id;

	const std::vector< Material >& materials = model.getMaterials();
	for (std::vector< Material >::const_iterator i = materials.begin(); i != materials.end(); ++i)
		cleaned.addMaterial(*i);

	const std::vector< Polygon >& polygons = model.getPolygons();
	for (std::vector< Polygon >::const_iterator i = polygons.begin(); i != polygons.end(); ++i)
	{
		Polygon cleanedPolygon;

		cleanedPolygon.setMaterial(i->getMaterial());

		id = i->getNormal();
		if (id != c_InvalidIndex)
			cleanedPolygon.setNormal(cleaned.addUniqueNormal(model.getNormal(id).normalized()));

		const std::vector< uint32_t >& vertices = i->getVertices();
		for (std::vector< uint32_t >::const_iterator j = vertices.begin(); j != vertices.end(); ++j)
		{
			if (*j == c_InvalidIndex)
				continue;

			const Vertex& vertex = model.getVertex(*j);
			Vertex cleanedVertex;

			id = vertex.getPosition();
			if (id != c_InvalidIndex)
				cleanedVertex.setPosition(cleaned.addUniquePosition(model.getPosition(id), m_positionDistance));

			id = vertex.getColor();
			if (id != c_InvalidIndex)
				cleanedVertex.setColor(cleaned.addUniqueColor(model.getColor(id)));

			id = vertex.getNormal();
			if (id != c_InvalidIndex)
				cleanedVertex.setNormal(cleaned.addUniqueNormal(model.getNormal(id).normalized()));

			id = vertex.getTangent();
			if (id != c_InvalidIndex)
				cleanedVertex.setTangent(cleaned.addUniqueNormal(model.getNormal(id).normalized()));

			id = vertex.getBinormal();
			if (id != c_InvalidIndex)
				cleanedVertex.setBinormal(cleaned.addUniqueNormal(model.getNormal(id).normalized()));

			uint32_t texCoordCount = vertex.getTexCoordCount();
			for (uint32_t k = 0; k < texCoordCount; ++k)
			{
				id = vertex.getTexCoord(k);
				if (id != c_InvalidIndex)
					cleanedVertex.setTexCoord(k, cleaned.addUniqueTexCoord(model.getTexCoord(id)));
			}

			uint32_t influenceCount = vertex.getJointInfluenceCount();
			for (uint32_t k = 0; k < influenceCount; ++k)
			{
				float influence = vertex.getJointInfluence(k);
				if (influence > FUZZY_EPSILON)
					cleanedVertex.setJointInfluence(k, influence);
			}

			cleanedPolygon.addVertex(cleaned.addUniqueVertex(cleanedVertex));
		}

		cleaned.addUniquePolygon(cleanedPolygon);
	}

	model = cleaned;
	return true;
}

	}
}
