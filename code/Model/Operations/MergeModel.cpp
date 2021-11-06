#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Model/Model.h"
#include "Model/Operations/MergeModel.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.MergeModel", MergeModel, IModelOperation)

MergeModel::MergeModel(const Model& sourceModel, const Transform& sourceTransform, float positionDistance)
:	m_sourceModel(sourceModel)
,	m_sourceTransform(sourceTransform)
,	m_positionDistance(positionDistance)
{
}

bool MergeModel::apply(Model& model) const
{
	SmallMap< uint32_t, uint32_t > materialMap;
	SmallMap< uint32_t, uint32_t > channelMap;

	// Merge texture channels.
	for (uint32_t i = 0; i < m_sourceModel.getTexCoordChannels().size(); ++i)
	{
		uint32_t channel = model.addUniqueTexCoordChannel(m_sourceModel.getTexCoordChannels()[i]);
		channelMap[i] = channel;
	}

	// Merge materials.
	for (uint32_t i = 0; i < m_sourceModel.getMaterialCount(); ++i)
	{
		const Material& material = m_sourceModel.getMaterial(i);
		materialMap[i] = model.addUniqueMaterial(material);
	}

	// Merge geometry.
	const AlignedVector< Polygon >& sourcePolygons = m_sourceModel.getPolygons();
	const AlignedVector< Vertex >& sourceVertices = m_sourceModel.getVertices();

	AlignedVector< uint32_t > vertexMap;
	vertexMap.resize(sourceVertices.size(), c_InvalidIndex);

	model.reservePositions(model.getPositionCount() + m_sourceModel.getPositionCount());
	model.reserveColors(model.getColorCount() + m_sourceModel.getColorCount());
	model.reserveNormals(model.getNormalCount() + m_sourceModel.getNormalCount());
	model.reserveVertices(model.getVertexCount() + m_sourceModel.getVertexCount());

	for (size_t i = 0; i < sourceVertices.size(); ++i)
	{
		const Vertex& sourceVertex = sourceVertices[i];

		uint32_t position = sourceVertex.getPosition();
		if (position != c_InvalidIndex)
			position = model.addUniquePosition(m_sourceTransform * m_sourceModel.getPosition(position).xyz1(), m_positionDistance);

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
				v.setTexCoord(channelMap[j], texCoord);
			}
		}

		for (uint32_t j = 0; j < sourceVertex.getJointInfluenceCount(); ++j)
		{
			float influence = sourceVertex.getJointInfluence(j);
			v.setJointInfluence(j, influence);
		}

		vertexMap[i] = model.addUniqueVertex(v);
	}

	AlignedVector< Polygon > outputPolygons;
	outputPolygons.reserve(sourcePolygons.size());

	for (size_t i = 0; i < sourcePolygons.size(); ++i)
	{
		const Polygon& sourcePolygon = sourcePolygons[i];
		const auto& sourceVertices = sourcePolygon.getVertices();

		Polygon outputPolygon;
		outputPolygon.setMaterial(materialMap[sourcePolygon.getMaterial()]);

		if (sourcePolygon.getNormal() != c_InvalidIndex)
		{
			uint32_t normal = model.addUniqueNormal(m_sourceTransform * m_sourceModel.getNormal(sourcePolygon.getNormal()).xyz0());
			outputPolygon.setNormal(normal);
		}

		for (size_t j = 0; j < sourceVertices.size(); ++j)
			outputPolygon.addVertex(vertexMap[sourceVertices[j]]);

		outputPolygons.push_back(outputPolygon);
	}

	AlignedVector< Polygon >& mergedPolygons = model.getPolygons();
	mergedPolygons.reserve(mergedPolygons.size() + sourcePolygons.size());
	mergedPolygons.insert(mergedPolygons.end(), outputPolygons.begin(), outputPolygons.end());
	return true;
}

	}
}
