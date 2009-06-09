#include <limits>
#include "Mesh/Editor/Blend/BlendMeshConverter.h"
#include "Mesh/Editor/ModelOptimizations.h"
#include "Mesh/Editor/MeshVertexWriter.h"
#include "Mesh/Blend/BlendMeshResource.h"
#include "Model/Utilities.h"
#include "Render/Mesh/SystemMeshFactory.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshWriter.h"
#include "Render/VertexBuffer.h"
#include "Render/IndexBuffer.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace mesh
	{

MeshResource* BlendMeshConverter::createResource() const
{
	return gc_new< BlendMeshResource >();
}

bool BlendMeshConverter::convert(
	const model::Model& sourceModel,
	const std::map< std::wstring, MeshPipelineParams::MaterialInfo >& materialInfo,
	const std::vector< render::VertexElement >& vertexElements,
	MeshResource* meshResource,
	Stream* meshResourceStream
) const
{
	Writer wr(meshResourceStream);

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
		if (i->getTexCoord() != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DuCustom, 0, model.getTexCoord(i->getTexCoord()));

		vertex += vertexSize;
	}

	baseMesh->getVertexBuffer()->unlock();

	// Create index buffer and build parts.
	std::vector< render::Mesh::Part > parts;
	std::vector< BlendMeshResource::Part > assetParts;

	uint16_t* index = static_cast< uint16_t* >(baseMesh->getIndexBuffer()->lock());
	uint16_t* indexFirst = index;

	for (std::vector< model::Material >::const_iterator j = model.getMaterials().begin(); j != model.getMaterials().end(); ++j)
	{
		const model::Material& material = *j;

		std::map< std::wstring, MeshPipelineParams::MaterialInfo >::const_iterator materialIt = materialInfo.find(material.getName());
		if (materialIt == materialInfo.end())
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

		parts.push_back(render::Mesh::Part());
		parts.back().name = material.getName();
		parts.back().primitives.setIndexed(
			render::PtTriangles,
			offset,
			triangleCount,
			minIndex,
			maxIndex
		);

		assetParts.push_back(BlendMeshResource::Part());
		assetParts.back().name = material.getName();
		assetParts.back().material = materialIt->second.guid;
	}

	baseMesh->getIndexBuffer()->unlock();
	baseMesh->setParts(parts);
	baseMesh->setBoundingBox(model::calculateModelBoundingBox(model));

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
			if (j->getTexCoord() != model::c_InvalidIndex)
				writeVertexData(vertexElements, vertex, render::DuCustom, 0, Vector4(0.0f, 0.0f, 0.0f, 0.0f));

			vertex += vertexSize;
		}

		targetMesh->getVertexBuffer()->unlock();

		if (!render::MeshWriter().write(meshResourceStream, targetMesh))
			return false;
	}

	checked_type_cast< BlendMeshResource* >(meshResource)->setParts(assetParts);

	// Create blend shape "name to weight" mapping.
	for (uint32_t i = 0; i < model.getBlendTargetCount(); ++i)
		checked_type_cast< BlendMeshResource* >(meshResource)->setBlendTarget(model.getBlendTarget(i), i);

	return true;
}

	}
}
