#include "Model/Model.h"
#include "Model/Operations/MergeModel.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.MergeModel", MergeModel, IModelOperation)

MergeModel::MergeModel(const Model& sourceModel, const Transform& sourceTransform)
:	m_sourceModel(sourceModel)
,	m_sourceTransform(sourceTransform)
{
}

bool MergeModel::apply(Model& model) const
{
	std::map< uint32_t, uint32_t > materialMap;

	// Merge materials.
	const std::vector< Material >& sourceMaterials = m_sourceModel.getMaterials();
	for (uint32_t i = 0; i < sourceMaterials.size(); ++i)
		materialMap[i] = model.addUniqueMaterial(sourceMaterials[i]);

	// Merge geometry.
	const std::vector< Vertex >& sourceVertices = m_sourceModel.getVertices();

	std::vector< uint32_t > vertexMap;
	vertexMap.resize(sourceVertices.size(), c_InvalidIndex);

	for (size_t i = 0; i < sourceVertices.size(); ++i)
	{
		const Vertex& sourceVertex = sourceVertices[i];

		uint32_t position = sourceVertex.getPosition();
		if (position != c_InvalidIndex)
			position = model.addUniquePosition(m_sourceTransform * m_sourceModel.getPosition(position).xyz1());

		uint32_t color = sourceVertex.getColor();
		if (color != c_InvalidIndex)
			color = model.addUniqueColor(m_sourceModel.getColor(color));

		uint32_t normal = sourceVertex.getNormal();
		if (normal != c_InvalidIndex)
			normal = model.addUniqueNormal(m_sourceTransform * m_sourceModel.getNormal(normal).xyz0());

		uint32_t tangent = sourceVertex.getTangent();
		if (tangent != c_InvalidIndex)
			tangent = model.addUniqueNormal(m_sourceTransform * m_sourceModel.getNormal(tangent).xyz0());

		uint32_t binormal = sourceVertex.getBinormal();
		if (binormal != c_InvalidIndex)
			binormal = model.addUniqueNormal(m_sourceTransform * m_sourceModel.getNormal(binormal).xyz0());

		Vertex v;
		v.setPosition(position);
		v.setColor(color);
		v.setNormal(normal);
		v.setTangent(tangent);
		v.setBinormal(binormal);

		for (uint32_t j = 0; j < sourceVertex.getTexCoordCount(); ++j)
		{
			uint32_t texCoord = sourceVertex.getTexCoord(j);
			if (texCoord != c_InvalidIndex)
			{
				texCoord = model.addUniqueTexCoord(m_sourceModel.getTexCoord(texCoord));
				v.setTexCoord(j, texCoord);
			}
		}

		for (uint32_t j = 0; j < sourceVertex.getJointInfluenceCount(); ++j)
		{
			float influence = sourceVertex.getJointInfluence(j);
			v.setJointInfluence(j, influence);
		}

		vertexMap[i] = model.addUniqueVertex(v);
	}

	// Merge polygons.
	const std::vector< Polygon >& sourcePolygons = m_sourceModel.getPolygons();
	for (std::vector< Polygon >::const_iterator i = sourcePolygons.begin(); i != sourcePolygons.end(); ++i)
	{
		Polygon p;
		p.setMaterial(materialMap[i->getMaterial()]);

		const std::vector< uint32_t >& sourceVertices = i->getVertices();
		for (uint32_t j = 0; j < sourceVertices.size(); ++j)
			p.addVertex(vertexMap[sourceVertices[j]]);

		if (i->getNormal() != c_InvalidIndex)
		{
			uint32_t normal = model.addUniqueNormal(m_sourceTransform * m_sourceModel.getNormal(i->getNormal()).xyz0());
			p.setNormal(normal);
		}

		model.addUniquePolygon(p);
	}

	return true;
}

	}
}
