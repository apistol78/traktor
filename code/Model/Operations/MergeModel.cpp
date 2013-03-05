#include "Core/Log/Log.h"
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

	std::vector< Polygon > mergedPolygons = model.getPolygons();
	mergedPolygons.reserve(mergedPolygons.size() + sourcePolygons.size());

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

		// Check if polygon exist with opposite winding; if such then the both
		// polygons is removed. Also don't add duplicates.
		bool duplicate = false;

		std::vector< uint32_t > pv = p.getVertices();
		std::reverse(pv.begin(), pv.end());

		for (std::vector< Polygon >::iterator i = mergedPolygons.begin(); i != mergedPolygons.end(); ++i)
		{
			if (i->getVertices() == p.getVertices())
			{
				duplicate = true;
				break;
			}

			if (i->getVertexCount() != p.getVertexCount() || i->getVertexCount() == 0)
				continue;

			uint32_t c = i->getVertexCount();
			uint32_t v0 = i->getVertex(0);
			uint32_t p0 = model.getVertex(v0).getPosition();

			uint32_t i0 = 0;
			for (; i0 < pv.size(); ++i0)
			{
				if (model.getVertex(pv[i0]).getPosition() == p0)
					break;
			}
			if (i0 >= pv.size())
				continue;

			bool cull = true;
			for (uint32_t j = 0; j < c; ++j)
			{
				const Vertex& v0 = model.getVertex(i->getVertex(j));
				const Vertex& v1 = model.getVertex(pv[(i0 + j) % c]);

				const Vector4& p0 = model.getPosition(v0.getPosition());
				const Vector4& p1 = model.getPosition(v1.getPosition());

				if ((p1 - p0).length2() > 0.01f * 0.01f)
				{
					cull = false;
					break;
				}
			}

			if (cull)
			{
				mergedPolygons.erase(i);
				duplicate = true;
				break;
			}
		}

		if (!duplicate)
			mergedPolygons.push_back(p);
	}

	model.setPolygons(mergedPolygons);

	return true;
}

	}
}
