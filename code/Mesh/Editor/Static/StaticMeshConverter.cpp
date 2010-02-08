#include <cstring>
#include <limits>
#include "Mesh/Editor/Static/StaticMeshConverter.h"
#include "Mesh/Editor/ModelOptimizations.h"
#include "Mesh/Editor/MeshVertexWriter.h"
#include "Mesh/Static/StaticMeshResource.h"
#include "Model/Model.h"
#include "Model/Utilities.h"
#include "Render/Mesh/SystemMeshFactory.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshWriter.h"
#include "Render/VertexBuffer.h"
#include "Render/IndexBuffer.h"
#include "Core/Math/Half.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace mesh
	{

Ref< MeshResource > StaticMeshConverter::createResource() const
{
	return new StaticMeshResource();
}

bool StaticMeshConverter::convert(
	const RefArray< model::Model >& models,
	const std::map< std::wstring, MaterialInfo >& materialInfo,
	const std::vector< render::VertexElement >& vertexElements,
	MeshResource* meshResource,
	IStream* meshResourceStream
) const
{
	// Create a copy of the first source model and triangulate it.
	model::Model model = *models[0];

	log::info << L"Triangulating model..." << Endl;
	model::triangulateModel(model);

	log::info << L"Sorting materials..." << Endl;
	sortMaterialsByProjectedArea(model);

	log::info << L"Sorting indices..." << Endl;
	model::sortPolygonsCacheCoherent(model);

	log::info << L"Calculating tangent bases..." << Endl;
	model::calculateModelTangents(model, true);

	log::info << L"Flatten materials..." << Endl;
	model::flattenDoubleSided(model);

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
			writeVertexData(vertexElements, vertex, render::DuCustom, 0, model.getTexCoord(i->getTexCoord(0)));
		if (i->getTexCoord(1) != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DuCustom, 1, model.getTexCoord(i->getTexCoord(1)));

		vertex += vertexSize;
	}

	mesh->getVertexBuffer()->unlock();

	// Create index buffer and build parts.
	std::vector< render::Mesh::Part > parts;
	std::vector< StaticMeshResource::Part > assetParts;

	uint16_t* index = static_cast< unsigned short* >(mesh->getIndexBuffer()->lock());
	uint16_t* indexFirst = index;

	for (std::vector< model::Material >::const_iterator i = model.getMaterials().begin(); i != model.getMaterials().end(); ++i)
	{
		const model::Material& material = *i;

		std::map< std::wstring, MaterialInfo >::const_iterator materialIt = materialInfo.find(material.getName());
		if (materialIt == materialInfo.end())
			continue;

		int offset = int(index - indexFirst);
		int triangleCount = 0;

		int minIndex =  std::numeric_limits< int >::max();
		int maxIndex = -std::numeric_limits< int >::max();

		for (std::vector< model::Polygon >::const_iterator j = model.getPolygons().begin(); j != model.getPolygons().end(); ++j)
		{
			const model::Polygon& polygon = *j;
			T_ASSERT (polygon.getVertices().size() == 3);

			if (polygon.getMaterial() != std::distance(model.getMaterials().begin(), i))
				continue;

			for (int k = 0; k < 3; ++k)
			{
				*index++ = uint16_t(polygon.getVertex(k));
				minIndex = std::min< int >(minIndex, polygon.getVertex(k));
				maxIndex = std::max< int >(maxIndex, polygon.getVertex(k));
			}

			triangleCount++;
		}

		if (!triangleCount)
			continue;

		parts.push_back(render::Mesh::Part());
		parts.back().name = material.getName();
		parts.back().primitives.setIndexed(
			render::PtTriangles,
			offset,
			triangleCount,
			minIndex,
			maxIndex
		);

		assetParts.push_back(StaticMeshResource::Part());
		assetParts.back().name = material.getName();
		assetParts.back().material = materialIt->second.guid;
		assetParts.back().opaque = materialIt->second.opaque;
	}

	mesh->getIndexBuffer()->unlock();
	mesh->setParts(parts);
	mesh->setBoundingBox(model::calculateModelBoundingBox(model));

	if (!render::MeshWriter().write(meshResourceStream, mesh))
		return false;

	checked_type_cast< StaticMeshResource* >(meshResource)->setParts(assetParts);

	return true;
}

	}
}
