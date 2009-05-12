#include <limits>
#include "Mesh/Editor/Instance/InstanceMeshConverter.h"
#include "Mesh/Editor/ModelOptimizations.h"
#include "Mesh/Editor/MeshVertexWriter.h"
#include "Mesh/Instance/InstanceMeshResource.h"
#include "Mesh/Instance/InstanceMesh.h"
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

MeshResource* InstanceMeshConverter::createResource() const
{
	return gc_new< InstanceMeshResource >();
}

bool InstanceMeshConverter::convert(
	const model::Model& sourceModel,
	const std::map< std::wstring, MeshPipelineParams::MaterialInfo >& materialInfo,
	const std::vector< render::VertexElement >& vertexElements,
	MeshResource* meshResource,
	Stream* meshResourceStream
) const
{
	// Create a copy of the source model and triangulate it.
	model::Model model = sourceModel;

	log::info << L"Triangulating model..." << Endl;
	model::triangulateModel(model);

	log::info << L"Sorting indices..." << Endl;
	model::sortPolygonsCacheCoherent(model);

	log::info << L"Calculating tangent bases..." << Endl;
	model::calculateModelTangents(model, true);

	log::info << L"Sorting materials..." << Endl;
	sortMaterialsByProjectedArea(model);

	log::info << L"Flatten materials..." << Endl;
	flattenDoubleSided(model);

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
		if (i->getTexCoord() != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DuCustom, 0, model.getTexCoord(i->getTexCoord()));

		vertex += vertexSize;
	}

	mesh->getVertexBuffer()->unlock();

	// Create index buffer and build parts.
	std::vector< render::Mesh::Part > parts;
	std::vector< InstanceMeshResource::Part > assetParts;

	uint16_t* index = static_cast< uint16_t* >(mesh->getIndexBuffer()->lock());
	uint16_t* indexFirst = index;

	for (std::vector< model::Material >::const_iterator i = model.getMaterials().begin(); i != model.getMaterials().end(); ++i)
	{
		const model::Material& material = *i;

		std::map< std::wstring, MeshPipelineParams::MaterialInfo >::const_iterator materialIt = materialInfo.find(material.getName());
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

		assetParts.push_back(InstanceMeshResource::Part());
		assetParts.back().name = material.getName();
		assetParts.back().material = materialIt->second.guid;
	}

	mesh->getIndexBuffer()->unlock();
	mesh->setParts(parts);
	mesh->setBoundingBox(model::calculateModelBoundingBox(model));

	if (!render::MeshWriter().write(meshResourceStream, mesh))
		return false;

	checked_type_cast< InstanceMeshResource* >(meshResource)->setParts(assetParts);

	return true;
}

	}
}
