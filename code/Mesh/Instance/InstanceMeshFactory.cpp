#include <cstring>
#include "Mesh/Instance/InstanceMeshFactory.h"
#include "Mesh/Instance/InstanceMeshResource.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Resource/IResourceManager.h"
#include "Render/Mesh/SystemMeshFactory.h"
#include "Render/Mesh/RenderMeshFactory.h"
#include "Render/Mesh/MeshReader.h"
#include "Render/Mesh/Mesh.h"
#include "Render/VertexBuffer.h"
#include "Render/IndexBuffer.h"
#include "Render/ITexture.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.InstanceMeshFactory", InstanceMeshFactory, resource::IResourceFactory)

InstanceMeshFactory::InstanceMeshFactory(db::Database* database, render::IRenderSystem* renderSystem, render::MeshFactory* meshFactory)
:	m_database(database)
,	m_renderSystem(renderSystem)
,	m_meshFactory(meshFactory)
{
	if (!m_meshFactory)
		m_meshFactory = new render::RenderMeshFactory(m_renderSystem);
}

const TypeInfoSet InstanceMeshFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< InstanceMesh >());
	return typeSet;
}

bool InstanceMeshFactory::isCacheable() const
{
	return true;
}

Ref< Object > InstanceMeshFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid)
{
	// Read single instance mesh.
	render::SystemMeshFactory systemMeshFactory;

	Ref< db::Instance > instance = m_database->getInstance(guid);
	if (!instance)
	{
		log::error << L"Instance mesh factory failed; no such instance" << Endl;
		return 0;
	}

	Ref< InstanceMeshResource > meshResource = instance->getObject< InstanceMeshResource >();
	if (!meshResource)
	{
		log::error << L"Instance mesh factory failed; unable to checkout resource" << Endl;
		return 0;
	}

	Ref< IStream > dataStream = instance->readData(L"Data");
	if (!dataStream)
	{
		log::error << L"Instance mesh factory failed; unable to open data stream" << Endl;
		return 0;
	}

	Ref< render::Mesh > singleInstanceMesh = render::MeshReader(&systemMeshFactory).read(dataStream);
	if (!singleInstanceMesh)
	{
		log::error << L"Instance mesh factory failed; unable to read mesh" << Endl;
		return 0;
	}
	
	dataStream->close();

	uint32_t vertexSize = render::getVertexSize(singleInstanceMesh->getVertexElements());
	uint32_t vertexCount = singleInstanceMesh->getVertexBuffer()->getBufferSize() / vertexSize;
	uint32_t instanceOffset = findVertexElement(singleInstanceMesh->getVertexElements(), render::DuCustom, 1)->getOffset();
	
	T_ASSERT (vertexSize * vertexCount == singleInstanceMesh->getVertexBuffer()->getBufferSize());
	T_ASSERT (singleInstanceMesh->getIndexBuffer()->getIndexType() == render::ItUInt16);

	render::IndexType indexType = vertexCount * InstanceMesh::MaxInstanceCount > 65535 ? render::ItUInt32 : render::ItUInt16;

	// Create instancing mesh.
	Ref< render::Mesh > renderMesh = m_meshFactory->createMesh(
		singleInstanceMesh->getVertexElements(),
		singleInstanceMesh->getVertexBuffer()->getBufferSize() * InstanceMesh::MaxInstanceCount,
		indexType,
		singleInstanceMesh->getIndexBuffer()->getBufferSize() * InstanceMesh::MaxInstanceCount * (indexType == render::ItUInt16 ? 1 : 2)
	);

	// Fill instancing vertex buffer.
	uint8_t* destinationVertex = static_cast< uint8_t* >(renderMesh->getVertexBuffer()->lock());
	for (uint32_t i = 0; i < InstanceMesh::MaxInstanceCount; ++i)
	{
		const uint8_t* sourceVertex = static_cast< const uint8_t* >(singleInstanceMesh->getVertexBuffer()->lock());
		for (uint32_t j = 0; j < singleInstanceMesh->getVertexBuffer()->getBufferSize(); j += vertexSize)
		{
			T_ASSERT (j / vertexSize < vertexCount);

			// Copy vertex, patch instance index.
			memcpy(destinationVertex, sourceVertex, vertexSize);
			*reinterpret_cast< float* >(destinationVertex + instanceOffset) = float(i);

			destinationVertex += vertexSize;
			sourceVertex += vertexSize;
		}
		singleInstanceMesh->getVertexBuffer()->unlock();
	}
	renderMesh->getVertexBuffer()->unlock();

	// Fill instancing index buffer.
	const uint16_t* sourceIndex = static_cast< const uint16_t* >(singleInstanceMesh->getIndexBuffer()->lock());
	
	uint8_t* destinationIndex = static_cast< uint8_t* >(renderMesh->getIndexBuffer()->lock());
	uint8_t* destinationIndexTop = destinationIndex;
	uint32_t destinationIndexSize = indexType == render::ItUInt16 ? 2 : 4;

	const std::vector< render::Mesh::Part >& singleInstanceParts = singleInstanceMesh->getParts();
	std::vector< render::Mesh::Part > renderParts(singleInstanceParts.size());
	
	for (uint32_t i = 0; i < singleInstanceParts.size(); ++i)
	{
		const render::Mesh::Part& singleInstancePart = singleInstanceParts[i];

		renderParts[i].name = singleInstancePart.name;
		renderParts[i].primitives = singleInstancePart.primitives;
		renderParts[i].primitives.offset = uint32_t(destinationIndex - destinationIndexTop) / destinationIndexSize;
		renderParts[i].primitives.minIndex = singleInstancePart.primitives.minIndex;
		renderParts[i].primitives.maxIndex = singleInstancePart.primitives.maxIndex + (InstanceMesh::MaxInstanceCount - 1) * vertexCount;

		for (uint32_t j = 0; j < InstanceMesh::MaxInstanceCount; ++j)
		{
			uint32_t vertexOffset = j * vertexCount;
			if (indexType == render::ItUInt16)
			{
				for (uint32_t k = 0; k < singleInstancePart.primitives.count; ++k)
				{
					reinterpret_cast< uint16_t* >(destinationIndex)[0] = sourceIndex[singleInstancePart.primitives.offset + k * 3 + 0] + vertexOffset;
					reinterpret_cast< uint16_t* >(destinationIndex)[1] = sourceIndex[singleInstancePart.primitives.offset + k * 3 + 1] + vertexOffset;
					reinterpret_cast< uint16_t* >(destinationIndex)[2] = sourceIndex[singleInstancePart.primitives.offset + k * 3 + 2] + vertexOffset;
					destinationIndex += 3 * sizeof(uint16_t);
				}
			}
			else	// render::ItUInt32
			{
				for (uint32_t k = 0; k < singleInstancePart.primitives.count; ++k)
				{
					reinterpret_cast< uint32_t* >(destinationIndex)[0] = sourceIndex[singleInstancePart.primitives.offset + k * 3 + 0] + vertexOffset;
					reinterpret_cast< uint32_t* >(destinationIndex)[1] = sourceIndex[singleInstancePart.primitives.offset + k * 3 + 1] + vertexOffset;
					reinterpret_cast< uint32_t* >(destinationIndex)[2] = sourceIndex[singleInstancePart.primitives.offset + k * 3 + 2] + vertexOffset;
					destinationIndex += 3 * sizeof(uint32_t);
				}
			}
		}
	}
	renderMesh->setParts(renderParts);
	renderMesh->getIndexBuffer()->unlock();
	singleInstanceMesh->getIndexBuffer()->unlock();

	renderMesh->setBoundingBox(singleInstanceMesh->getBoundingBox());

	const std::vector< InstanceMeshResource::Part >& parts = meshResource->getParts();
	if (parts.size() != renderMesh->getParts().size())
	{
		log::error << L"Instance mesh factory failed; parts mismatch" << Endl;
		return 0;
	}

	Ref< InstanceMesh > instanceMesh = new InstanceMesh();
	instanceMesh->m_mesh = renderMesh;
	instanceMesh->m_parts.resize(parts.size());

	for (size_t i = 0; i < parts.size(); ++i)
	{
		instanceMesh->m_parts[i].material = parts[i].material;
		if (!resourceManager->bind(instanceMesh->m_parts[i].material))
			return 0;
	}

	return instanceMesh;
}

	}
}
