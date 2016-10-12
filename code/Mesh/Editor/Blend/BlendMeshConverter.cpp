#include <cstring>
#include <limits>
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Mesh/Blend/BlendMeshResource.h"
#include "Mesh/Editor/MeshVertexWriter.h"
#include "Mesh/Editor/Blend/BlendMeshConverter.h"
#include "Model/Model.h"
#include "Model/Operations/CalculateTangents.h"
#include "Model/Operations/FlattenDoubleSided.h"
#include "Model/Operations/SortCacheCoherency.h"
#include "Model/Operations/SortProjectedArea.h"
#include "Model/Operations/Triangulate.h"
#include "Render/Mesh/SystemMeshFactory.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshWriter.h"
#include "Render/VertexBuffer.h"
#include "Render/IndexBuffer.h"

namespace traktor
{
	namespace mesh
	{

Ref< IMeshResource > BlendMeshConverter::createResource() const
{
	return new BlendMeshResource();
}

bool BlendMeshConverter::convert(
	const MeshAsset* meshAsset,
	const RefArray< model::Model >& models,
	const Guid& materialGuid,
	const std::map< std::wstring, std::list< MeshMaterialTechnique > >& materialTechniqueMap,
	const std::vector< render::VertexElement >& vertexElements,
	int32_t maxInstanceCount,
	IMeshResource* meshResource,
	IStream* meshResourceStream
) const
{
	Writer wr(meshResourceStream);

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

	// Write number of blend shapes in data stream.
	uint32_t meshCount = 1 + model.getBlendTargetCount();
	wr << meshCount;

	// Create primary blend shape.
	uint32_t vertexCount = uint32_t(model.getVertices().size());
	uint32_t triangleCount = uint32_t(model.getPolygons().size());
	uint32_t vertexBufferSize = vertexCount * vertexSize;
	uint32_t indexBufferSize = triangleCount * 3 * sizeof(uint16_t);

	Ref< render::Mesh > baseMesh = render::SystemMeshFactory().createMesh(
		vertexElements,
		vertexBufferSize,
		render::ItUInt16,
		indexBufferSize
	);

	// Create vertex buffer.
	uint8_t* vertex = static_cast< uint8_t* >(baseMesh->getVertexBuffer()->lock());

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

	baseMesh->getVertexBuffer()->unlock();

	// Create index buffer and build parts.
	std::vector< render::Mesh::Part > meshParts;
	std::map< std::wstring, BlendMeshResource::parts_t > parts;

	uint16_t* index = static_cast< uint16_t* >(baseMesh->getIndexBuffer()->lock());
	uint16_t* indexFirst = index;

	for (std::vector< model::Material >::const_iterator j = model.getMaterials().begin(); j != model.getMaterials().end(); ++j)
	{
		const model::Material& material = *j;

		std::map< std::wstring, std::list< MeshMaterialTechnique > >::const_iterator materialIt = materialTechniqueMap.find(material.getName());
		if (materialIt == materialTechniqueMap.end())
			continue;

		int offset = int(index - indexFirst);
		int triangleCount = 0;

		int minIndex =  std::numeric_limits< int >::max();
		int maxIndex = -std::numeric_limits< int >::max();

		for (std::vector< model::Polygon >::const_iterator k = model.getPolygons().begin(); k != model.getPolygons().end(); ++k)
		{
			const model::Polygon& polygon = *k;
			T_ASSERT (polygon.getVertices().size() == 3);

			if (polygon.getMaterial() != std::distance(model.getMaterials().begin(), j))
				continue;

			for (int m = 0; m < 3; ++m)
			{
				*index++ = uint16_t(polygon.getVertex(m));
				minIndex = std::min< int >(minIndex, polygon.getVertex(m));
				maxIndex = std::max< int >(maxIndex, polygon.getVertex(m));
			}

			triangleCount++;
		}

		if (!triangleCount)
			continue;

		for (std::list< MeshMaterialTechnique >::const_iterator j = materialIt->second.begin(); j != materialIt->second.end(); ++j)
		{
			BlendMeshResource::Part part;
			part.shaderTechnique = j->shaderTechnique;
			part.meshPart = uint32_t(meshParts.size());
			parts[j->worldTechnique].push_back(part);
		}

		render::Mesh::Part meshPart;
		meshPart.name = material.getName();
		meshPart.primitives.setIndexed(
			render::PtTriangles,
			offset,
			triangleCount,
			minIndex,
			maxIndex
		);
		meshParts.push_back(meshPart);
	}

	baseMesh->getIndexBuffer()->unlock();
	baseMesh->setParts(meshParts);
	baseMesh->setBoundingBox(model.getBoundingBox());

	if (!render::MeshWriter().write(meshResourceStream, baseMesh))
		return false;

	// Create target blend shape meshes, only vertices are important.
	for (uint32_t i = 1; i < meshCount; ++i)
	{
		Ref< render::Mesh > targetMesh = render::SystemMeshFactory().createMesh(
			vertexElements,
			vertexBufferSize,
			render::ItUInt16,
			0
		);

		// Create vertex buffer.
		uint8_t* vertex = static_cast< uint8_t* >(targetMesh->getVertexBuffer()->lock());

		for (std::vector< model::Vertex >::const_iterator j = model.getVertices().begin(); j != model.getVertices().end(); ++j)
		{
			std::memset(vertex, 0, vertexSize);

			Vector4 basePosition = model.getPosition(j->getPosition());
			Vector4 targetPosition = model.getBlendTargetPosition(i - 1, j->getPosition());
			Vector4 deltaPosition = targetPosition - basePosition;

			writeVertexData(vertexElements, vertex, render::DuPosition, 0, deltaPosition);

			if (j->getNormal() != model::c_InvalidIndex)
				writeVertexData(vertexElements, vertex, render::DuNormal, 0, Vector4(0.0f, 0.0f, 0.0f, 0.0f));
			if (j->getTangent() != model::c_InvalidIndex)
				writeVertexData(vertexElements, vertex, render::DuTangent, 0, Vector4(0.0f, 0.0f, 0.0f, 0.0f));
			if (j->getBinormal() != model::c_InvalidIndex)
				writeVertexData(vertexElements, vertex, render::DuBinormal, 0, Vector4(0.0f, 0.0f, 0.0f, 0.0f));
			if (j->getTexCoord(0) != model::c_InvalidIndex)
				writeVertexData(vertexElements, vertex, render::DuCustom, 0, Vector4(0.0f, 0.0f, 0.0f, 0.0f));
			if (j->getTexCoord(1) != model::c_InvalidIndex)
				writeVertexData(vertexElements, vertex, render::DuCustom, 1, Vector4(0.0f, 0.0f, 0.0f, 0.0f));

			vertex += vertexSize;
		}

		targetMesh->getVertexBuffer()->unlock();

		if (!render::MeshWriter().write(meshResourceStream, targetMesh))
			return false;
	}

	checked_type_cast< BlendMeshResource* >(meshResource)->m_shader = resource::Id< render::Shader >(materialGuid);
	checked_type_cast< BlendMeshResource* >(meshResource)->m_parts = parts;

	// Create blend shape "name to weight" mapping.
	for (uint32_t i = 0; i < model.getBlendTargetCount(); ++i)
		checked_type_cast< BlendMeshResource* >(meshResource)->m_targetMap[model.getBlendTarget(i)] = i;

	return true;
}

	}
}
