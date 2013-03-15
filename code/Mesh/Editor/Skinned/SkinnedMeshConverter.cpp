#include <algorithm>
#include <cstring>
#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Half.h"
#include "Core/Misc/String.h"
#include "Mesh/Editor/IndexRange.h"
#include "Mesh/Editor/MeshVertexWriter.h"
#include "Mesh/Editor/Skinned/SkinnedMeshConverter.h"
#include "Mesh/Skinned/SkinnedMeshResource.h"
#include "Model/Model.h"
#include "Model/Operations/CalculateTangents.h"
#include "Model/Operations/FlattenDoubleSided.h"
#include "Model/Operations/SortCacheCoherency.h"
#include "Model/Operations/SortProjectedArea.h"
#include "Model/Operations/Triangulate.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshWriter.h"
#include "Render/Mesh/SystemMeshFactory.h"
#include "Render/VertexBuffer.h"
#include "Render/IndexBuffer.h"

namespace traktor
{
	namespace mesh
	{
		namespace
		{

struct InfluencePredicate
{
	bool operator () (const std::pair< int, float >& lh, const std::pair< int, float >& rh) const
	{
		return lh.second > rh.second;
	}
};

		}

Ref< IMeshResource > SkinnedMeshConverter::createResource() const
{
	return new SkinnedMeshResource();
}

bool SkinnedMeshConverter::convert(
	const RefArray< model::Model >& models,
	const model::Model* occluderModel,
	const Guid& materialGuid,
	const std::map< std::wstring, std::list< MeshMaterialTechnique > >& materialTechniqueMap,
	const std::vector< render::VertexElement >& vertexElements,
	IMeshResource* meshResource,
	IStream* meshResourceStream
) const
{
	// Create a copy of the first source model and triangulate it.
	model::Model model = *models[0];

	log::info << L"Triangulating model..." << Endl;
	model::Triangulate().apply(model);

	log::info << L"Sorting indices..." << Endl;
	model::SortCacheCoherency().apply(model);

	log::info << L"Calculating tangent bases..." << Endl;
	model::CalculateTangents().apply(model);

	log::info << L"Sorting materials..." << Endl;
	model::SortProjectedArea(false).apply(model);

	log::info << L"Flatten materials..." << Endl;
	model::FlattenDoubleSided().apply(model);

	// Create vertex declaration.
	log::info << L"Creating mesh..." << Endl;

	uint32_t vertexSize = render::getVertexSize(vertexElements);
	T_ASSERT (vertexSize > 0);

	// Create render mesh.
	uint32_t vertexBufferSize = uint32_t(model.getVertices().size() * vertexSize);
	uint32_t indexBufferSize = uint32_t(model.getPolygons().size() * 3 * sizeof(uint16_t));

	Ref< render::Mesh > mesh = render::SystemMeshFactory().createMesh(
		vertexElements,
		vertexBufferSize,
		render::ItUInt16,
		indexBufferSize
	);

	// Create vertex buffer.
	uint8_t* vertex = static_cast< uint8_t* >(mesh->getVertexBuffer()->lock());

	for (std::vector< model::Vertex >::const_iterator i = model.getVertices().begin(); i != model.getVertices().end(); ++i)
	{
		std::memset(vertex, 0, vertexSize);

		writeVertexData(vertexElements, vertex, render::DuPosition, 0, model.getPosition(i->getPosition()));
		if (i->getNormal() != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DuNormal, 0, model.getNormal(i->getNormal()));
		if (i->getTangent() != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DuTangent, 0, model.getNormal(i->getTangent()));
		if (i->getBinormal() != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DuBinormal, 0, model.getNormal(i->getBinormal()));
		if (i->getColor() != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DuColor, 0, model.getColor(i->getColor()));
		if (i->getTexCoord(0) != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DuCustom, 2, model.getTexCoord(i->getTexCoord(0)));
		if (i->getTexCoord(1) != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DuCustom, 3, model.getTexCoord(i->getTexCoord(1)));

		int jointCount = model.getJointCount();

		std::vector< std::pair< int, float > > jointInfluences;
		for (int j = 0; j < jointCount; ++j)
			jointInfluences.push_back(std::make_pair(j, i->getJointInfluence(j)));

		std::sort(jointInfluences.begin(), jointInfluences.end(), InfluencePredicate());

		jointCount = std::min(4, jointCount);

		float totalInfluence = 0.0f;
		for (int j = 0; j < jointCount; ++j)
			totalInfluence += jointInfluences[j].second;

		float blendIndices[4], blendWeights[4];
		if (std::fabs(totalInfluence) > FUZZY_EPSILON)
		{
			// Don't normalize single bone vertices; skinned with world.
			if (jointCount <= 1)
				totalInfluence = 1.0f;

			for (int j = 0; j < jointCount; ++j)
			{
				blendIndices[j] = float(jointInfluences[j].first);
				blendWeights[j] = jointInfluences[j].second / totalInfluence;
			}

			for (int j = jointCount; j < 4; ++j)
			{
				blendIndices[j] =
				blendWeights[j] = 0.0f;
			}
		}
		else
		{
			for (int j = 0; j < 4; ++j)
			{
				blendIndices[j] =
				blendWeights[j] = 0.0f;
			}
		}

		writeVertexData(vertexElements, vertex, render::DuCustom, 0, blendIndices);
		writeVertexData(vertexElements, vertex, render::DuCustom, 1, blendWeights);

		vertex += vertexSize;
	}

	mesh->getVertexBuffer()->unlock();

	// Create index buffer.
	std::map< std::wstring, std::vector< IndexRange > > techniqueRanges;

	uint16_t* index = static_cast< uint16_t* >(mesh->getIndexBuffer()->lock());
	uint16_t* indexFirst = index;

	for (std::map< std::wstring, std::list< MeshMaterialTechnique > >::const_iterator i = materialTechniqueMap.begin(); i != materialTechniqueMap.end(); ++i)
	{
		IndexRange range;

		range.offsetFirst = int32_t(index - indexFirst);
		range.offsetLast = 0;
		range.minIndex = std::numeric_limits< int32_t >::max();
		range.maxIndex = -std::numeric_limits< int32_t >::max();

		for (std::vector< model::Polygon >::const_iterator j = model.getPolygons().begin(); j != model.getPolygons().end(); ++j)
		{
			const model::Polygon& polygon = *j;
			T_ASSERT (polygon.getVertices().size() == 3);

			if (model.getMaterial(polygon.getMaterial()).getName() != i->first)
				continue;

			for (int k = 0; k < 3; ++k)
			{
				*index++ = (uint16_t)(polygon.getVertex(k));
				range.minIndex = std::min< int32_t >(range.minIndex, polygon.getVertex(k));
				range.maxIndex = std::max< int32_t >(range.maxIndex, polygon.getVertex(k));
			}
		}

		range.offsetLast = int32_t(index - indexFirst);
		if (range.offsetLast <= range.offsetFirst)
			continue;

		for (std::list< MeshMaterialTechnique >::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			std::wstring technique = j->worldTechnique + L"/" + j->shaderTechnique;
			range.mergeInto(techniqueRanges[technique]);
		}
	}

	mesh->getIndexBuffer()->unlock();

	// Build parts.
	std::vector< render::Mesh::Part > meshParts;
	std::map< std::wstring, SkinnedMeshResource::parts_t > parts;

	for (std::map< std::wstring, std::vector< IndexRange > >::const_iterator i = techniqueRanges.begin(); i != techniqueRanges.end(); ++i)
	{
		std::wstring worldTechnique, shaderTechnique;
		split(i->first, L'/', worldTechnique, shaderTechnique);

		for (std::vector< IndexRange >::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			SkinnedMeshResource::Part part;
			part.shaderTechnique = shaderTechnique;
			part.meshPart = uint32_t(meshParts.size());

			for (uint32_t k = 0; k < uint32_t(meshParts.size()); ++k)
			{
				if (
					meshParts[k].primitives.offset == j->offsetFirst &&
					meshParts[k].primitives.count == (j->offsetLast - j->offsetFirst) / 3
				)
				{
					part.meshPart = k;
					break;
				}
			}

			if (part.meshPart >= meshParts.size())
			{
				render::Mesh::Part meshPart;
				meshPart.primitives.setIndexed(
					render::PtTriangles,
					j->offsetFirst,
					(j->offsetLast - j->offsetFirst) / 3,
					j->minIndex,
					j->maxIndex
				);
				meshParts.push_back(meshPart);
			}

			parts[worldTechnique].push_back(part);
		}
	}

	mesh->setParts(meshParts);
	mesh->setBoundingBox(model.getBoundingBox());

	if (!render::MeshWriter().write(meshResourceStream, mesh))
		return false;

	checked_type_cast< SkinnedMeshResource* >(meshResource)->m_shader = resource::Id< render::Shader >(materialGuid);
	checked_type_cast< SkinnedMeshResource* >(meshResource)->m_parts = parts;
	for (uint32_t i = 0; i < model.getJointCount(); ++i)
		checked_type_cast< SkinnedMeshResource* >(meshResource)->m_jointMap[model.getJoint(i)] = i;

	return true;
}

	}
}
