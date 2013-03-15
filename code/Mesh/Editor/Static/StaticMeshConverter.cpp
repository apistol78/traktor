#include <cstring>
#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Half.h"
#include "Core/Misc/String.h"
#include "Mesh/Editor/IndexRange.h"
#include "Mesh/Editor/MeshVertexWriter.h"
#include "Mesh/Editor/Static/StaticMeshConverter.h"
#include "Mesh/Static/StaticMeshResource.h"
#include "Model/Model.h"
#include "Model/Operations/CalculateTangents.h"
#include "Model/Operations/FlattenDoubleSided.h"
#include "Model/Operations/SortCacheCoherency.h"
#include "Model/Operations/SortProjectedArea.h"
#include "Model/Operations/Triangulate.h"
#include "Render/VertexBuffer.h"
#include "Render/IndexBuffer.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshWriter.h"
#include "Render/Mesh/SystemMeshFactory.h"
#include "World/SwHiZ/OccluderMesh.h"
#include "World/SwHiZ/OccluderMeshWriter.h"

namespace traktor
{
	namespace mesh
	{

Ref< IMeshResource > StaticMeshConverter::createResource() const
{
	return new StaticMeshResource();
}

bool StaticMeshConverter::convert(
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

	//---------------------------------------
	// Create occluder mesh.

	if (occluderModel)
	{
		log::info << L"Creating occluder mesh..." << Endl;

		Ref< world::OccluderMesh > occluderMesh = new world::OccluderMesh(
			occluderModel->getVertexCount(),
			occluderModel->getPolygonCount() * 3
		);

		float* occluderVertex = occluderMesh->getVertices();
		for (uint32_t i = 0; i < occluderModel->getVertexCount(); ++i)
		{
			const model::Vertex& vertex = occluderModel->getVertex(i);
			const Vector4& position = occluderModel->getPosition(vertex.getPosition());

			*occluderVertex++ = position.x();
			*occluderVertex++ = position.y();
			*occluderVertex++ = position.z();
			*occluderVertex++ = 1.0f;
		}

		uint16_t* occluderIndex = occluderMesh->getIndices();
		for (uint32_t i = 0; i < occluderModel->getPolygonCount(); ++i)
		{
			const model::Polygon& polygon = occluderModel->getPolygon(i);
			T_ASSERT (polygon.getVertexCount() == 3);

			*occluderIndex++ = uint16_t(polygon.getVertex(0));
			*occluderIndex++ = uint16_t(polygon.getVertex(1));
			*occluderIndex++ = uint16_t(polygon.getVertex(2));
		}

		world::OccluderMeshWriter().write(meshResourceStream, occluderMesh);
	}


	//---------------------------------------
	// Create render mesh.

	log::info << L"Creating render mesh..." << Endl;

	uint32_t vertexSize = render::getVertexSize(vertexElements);
	T_ASSERT (vertexSize > 0);

	bool useLargeIndices = bool(model.getVertexCount() >= 65536);
	uint32_t indexSize = useLargeIndices ? sizeof(uint32_t) : sizeof(uint16_t);

	if (useLargeIndices)
		log::warning << L"Using 32-bit indices; might not work on all renderers" << Endl;

	// Create render mesh.
	uint32_t vertexBufferSize = uint32_t(model.getVertices().size() * vertexSize);
	uint32_t indexBufferSize = uint32_t(model.getPolygons().size() * 3 * indexSize);

	Ref< render::Mesh > renderMesh = render::SystemMeshFactory().createMesh(
		vertexElements,
		vertexBufferSize,
		useLargeIndices ? render::ItUInt32 : render::ItUInt16,
		indexBufferSize
	);

	// Create vertex buffer.
	uint8_t* vertex = static_cast< uint8_t* >(renderMesh->getVertexBuffer()->lock());

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
			writeVertexData(vertexElements, vertex, render::DuCustom, 0, model.getTexCoord(i->getTexCoord(0)));
		if (i->getTexCoord(1) != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DuCustom, 1, model.getTexCoord(i->getTexCoord(1)));

		vertex += vertexSize;
	}

	renderMesh->getVertexBuffer()->unlock();

	// Create index buffer.
	std::map< std::wstring, std::vector< IndexRange > > techniqueRanges;

	uint8_t* index = static_cast< uint8_t* >(renderMesh->getIndexBuffer()->lock());
	uint8_t* indexFirst = index;

	for (std::map< std::wstring, std::list< MeshMaterialTechnique > >::const_iterator i = materialTechniqueMap.begin(); i != materialTechniqueMap.end(); ++i)
	{
		IndexRange range;
		
		range.offsetFirst = uint32_t(index - indexFirst) / indexSize;
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

		for (std::list< MeshMaterialTechnique >::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			std::wstring technique = j->worldTechnique + L"/" + j->shaderTechnique;
			range.mergeInto(techniqueRanges[technique]);
		}
	}

	renderMesh->getIndexBuffer()->unlock();

	// Dump index ranges.
	log::info << L"Index ranges" << Endl;
	log::info << IncreaseIndent;

	for (std::map< std::wstring, std::vector< IndexRange > >::const_iterator i = techniqueRanges.begin(); i != techniqueRanges.end(); ++i)
	{
		log::info << L"\"" << i->first << L"\"" << Endl;
	
		log::info << IncreaseIndent;
		for (uint32_t j = 0; j < i->second.size(); ++j)
		{
			const IndexRange& range = i->second[j];
			log::info << j << L". offset from " << range.offsetFirst << L" to " << range.offsetLast << L", index min " << range.minIndex << L" max " << range.maxIndex << Endl;
		}
		log::info << DecreaseIndent;
	}

	log::info << DecreaseIndent;

	// Build parts.
	std::vector< render::Mesh::Part > meshParts;
	std::map< std::wstring, StaticMeshResource::parts_t > parts;

	for (std::map< std::wstring, std::vector< IndexRange > >::const_iterator i = techniqueRanges.begin(); i != techniqueRanges.end(); ++i)
	{
		std::wstring worldTechnique, shaderTechnique;
		split(i->first, L'/', worldTechnique, shaderTechnique);

		for (std::vector< IndexRange >::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			StaticMeshResource::Part part;
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

	renderMesh->setParts(meshParts);
	renderMesh->setBoundingBox(model.getBoundingBox());

	if (!render::MeshWriter().write(meshResourceStream, renderMesh))
		return false;

	checked_type_cast< StaticMeshResource* >(meshResource)->m_haveRenderMesh = true;
	checked_type_cast< StaticMeshResource* >(meshResource)->m_haveOccluderMesh = bool(occluderModel != 0);
	checked_type_cast< StaticMeshResource* >(meshResource)->m_shader = resource::Id< render::Shader >(materialGuid);
	checked_type_cast< StaticMeshResource* >(meshResource)->m_parts = parts;

	return true;
}

	}
}
