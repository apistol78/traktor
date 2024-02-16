/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <cstring>
#include <limits>
#include "Core/Log/Log.h"
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
#include "Render/Buffer.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshWriter.h"
#include "Render/Mesh/SystemMeshFactory.h"

namespace traktor::mesh
{

Ref< MeshResource > SkinnedMeshConverter::createResource() const
{
	return new SkinnedMeshResource();
}

bool SkinnedMeshConverter::getOperations(const MeshAsset* meshAsset, bool editor, RefArray< const model::IModelOperation >& outOperations) const
{
	outOperations.push_back(new model::Triangulate());
	if (!editor)
		outOperations.push_back(new model::SortCacheCoherency());
	outOperations.push_back(new model::CalculateTangents(false));
	outOperations.push_back(new model::SortProjectedArea(false));
	outOperations.push_back(new model::FlattenDoubleSided());
	return true;
}

bool SkinnedMeshConverter::convert(
	const MeshAsset* meshAsset,
	const RefArray< model::Model >& models,
	const Guid& materialGuid,
	const std::map< std::wstring, std::list< MeshMaterialTechnique > >& materialTechniqueMap,
	const AlignedVector< render::VertexElement >& vertexElements,
	MeshResource* meshResource,
	IStream* meshResourceStream
) const
{
	const model::Model* model = models[0];
	T_FATAL_ASSERT(model != nullptr);

	// Create render mesh.
	uint32_t vertexSize = render::getVertexSize(vertexElements);
	T_ASSERT(vertexSize > 0);

	bool useLargeIndices = (bool)(model->getVertexCount() >= 65536);
	uint32_t indexSize = useLargeIndices ? sizeof(uint32_t) : sizeof(uint16_t);

	uint32_t vertexBufferSize = (uint32_t)(model->getVertices().size() * vertexSize);
	uint32_t indexBufferSize = (uint32_t)(model->getPolygons().size() * 3 * indexSize);

	Ref< render::Mesh > mesh = render::SystemMeshFactory().createMesh(
		vertexElements,
		vertexBufferSize,
		useLargeIndices ? render::IndexType::UInt32 : render::IndexType::UInt16,
		indexBufferSize
	);

	// Create vertex buffer.
	uint8_t* vertex = static_cast< uint8_t* >(mesh->getVertexBuffer()->lock());
	std::memset(vertex, 0, vertexBufferSize);

	AlignedVector< std::pair< uint32_t, float > > jointInfluences;
	for (const auto& v : model->getVertices())
	{
		writeVertexData(vertexElements, vertex, render::DataUsage::Position, 0, model->getPosition(v.getPosition()));
		if (v.getNormal() != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DataUsage::Normal, 0, model->getNormal(v.getNormal()));
		if (v.getTangent() != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DataUsage::Tangent, 0, model->getNormal(v.getTangent()));
		if (v.getBinormal() != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DataUsage::Binormal, 0, model->getNormal(v.getBinormal()));
		if (v.getColor() != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DataUsage::Color, 0, model->getColor(v.getColor()));
		if (v.getTexCoord(0) != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DataUsage::Custom, 2, model->getTexCoord(v.getTexCoord(0)));
		if (v.getTexCoord(1) != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DataUsage::Custom, 3, model->getTexCoord(v.getTexCoord(1)));

		uint32_t jointCount = model->getJointCount();

		jointInfluences.resize(0);
		for (uint32_t i = 0; i < jointCount; ++i)
		{
			float w = v.getJointInfluence(i);
			if (std::abs(w) > FUZZY_EPSILON)
				jointInfluences.push_back(std::make_pair(i, w));
		}

		std::sort(jointInfluences.begin(), jointInfluences.end(), [](const std::pair< int, float >& lh, const std::pair< int, float >& rh) {
			return lh.second > rh.second;
		});

		jointCount = (uint32_t)jointInfluences.size();
		jointCount = std::min< uint32_t >(4, jointCount);

		float totalInfluence = 0.0f;
		for (uint32_t j = 0; j < jointCount; ++j)
			totalInfluence += jointInfluences[j].second;

		float blendIndices[4], blendWeights[4];
		if (std::abs(totalInfluence) > FUZZY_EPSILON)
		{
			// Don't normalize single bone vertices; skinned with world.
			if (jointCount <= 1)
				totalInfluence = 1.0f;

			for (uint32_t i = 0; i < jointCount; ++i)
			{
				blendIndices[i] = (float)jointInfluences[i].first;
				blendWeights[i] = jointInfluences[i].second / totalInfluence;
			}

			for (uint32_t i = jointCount; i < 4; ++i)
			{
				blendIndices[i] =
				blendWeights[i] = 0.0f;
			}
		}
		else
		{
			for (uint32_t i = jointCount; i < 4; ++i)
			{
				blendIndices[i] =
				blendWeights[i] = 0.0f;
			}
		}

		writeVertexData(vertexElements, vertex, render::DataUsage::Custom, 0, blendIndices);
		writeVertexData(vertexElements, vertex, render::DataUsage::Custom, 1, blendWeights);

		vertex += vertexSize;
	}

	mesh->getVertexBuffer()->unlock();

	// Create index buffer.
	std::map< std::wstring, AlignedVector< IndexRange > > techniqueRanges;

	uint8_t* index = (uint8_t*)mesh->getIndexBuffer()->lock();
	uint8_t* indexFirst = index;

	for (const auto& mt : materialTechniqueMap)
	{
		IndexRange range;

		range.offsetFirst = uint32_t(index - indexFirst) / indexSize;
		range.offsetLast = 0;
		range.minIndex = std::numeric_limits< int32_t >::max();
		range.maxIndex = -std::numeric_limits< int32_t >::max();

		for (const auto& polygon : model->getPolygons())
		{
			T_ASSERT(polygon.getVertices().size() == 3);

			if (model->getMaterial(polygon.getMaterial()).getName() != mt.first)
				continue;

			for (int32_t k = 0; k < 3; ++k)
			{
				if (useLargeIndices)
					*(uint32_t*)index = polygon.getVertex(k);
				else
					*(uint16_t*)index = polygon.getVertex(k);

				range.minIndex = std::min< int32_t >(range.minIndex, polygon.getVertex(k));
				range.maxIndex = std::max< int32_t >(range.maxIndex, polygon.getVertex(k));

				index += indexSize;
			}
		}

		range.offsetLast = uint32_t(index - indexFirst) / indexSize;
		if (range.offsetLast <= range.offsetFirst)
			continue;

		for (const auto& mtt : mt.second)
		{
			std::wstring technique = mtt.worldTechnique + L"/" + mtt.shaderTechnique;
			range.mergeInto(techniqueRanges[technique]);
		}
	}

	mesh->getIndexBuffer()->unlock();

	// Build parts.
	AlignedVector< render::Mesh::Part > meshParts;
	SmallMap< std::wstring, SkinnedMeshResource::parts_t > parts;

	for (std::map< std::wstring, AlignedVector< IndexRange > >::const_iterator i = techniqueRanges.begin(); i != techniqueRanges.end(); ++i)
	{
		std::wstring worldTechnique, shaderTechnique;
		split(i->first, L'/', worldTechnique, shaderTechnique);

		for (AlignedVector< IndexRange >::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
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
					render::PrimitiveType::Triangles,
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
	mesh->setBoundingBox(model->getBoundingBox());

	if (!render::MeshWriter().write(meshResourceStream, mesh))
		return false;

	checked_type_cast< SkinnedMeshResource* >(meshResource)->m_shader = resource::Id< render::Shader >(materialGuid);
	checked_type_cast< SkinnedMeshResource* >(meshResource)->m_parts = parts;
	for (uint32_t i = 0; i < model->getJointCount(); ++i)
		checked_type_cast< SkinnedMeshResource* >(meshResource)->m_jointMap[model->getJoint(i).getName()] = i;

	return true;
}

}
