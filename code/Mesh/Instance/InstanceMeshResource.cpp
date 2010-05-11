#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStl.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Mesh/Instance/InstanceMeshResource.h"
#include "Render/IndexBuffer.h"
#include "Render/VertexBuffer.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshReader.h"
#include "Render/Mesh/SystemMeshFactory.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.InstanceMeshResource", 1, InstanceMeshResource, IMeshResource)

Ref< IMesh > InstanceMeshResource::createMesh(
	IStream* dataStream,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	render::MeshFactory* meshFactory
) const
{
	// Read single instance mesh.
	render::SystemMeshFactory systemMeshFactory;
	Ref< render::Mesh > singleInstanceMesh = render::MeshReader(&systemMeshFactory).read(dataStream);
	if (!singleInstanceMesh)
	{
		log::error << L"Instance mesh create failed; unable to read mesh" << Endl;
		return 0;
	}

	uint32_t vertexSize = render::getVertexSize(singleInstanceMesh->getVertexElements());
	uint32_t vertexCount = singleInstanceMesh->getVertexBuffer()->getBufferSize() / vertexSize;
	uint32_t instanceOffset = findVertexElement(singleInstanceMesh->getVertexElements(), render::DuCustom, 1)->getOffset();

	T_ASSERT (vertexSize * vertexCount == singleInstanceMesh->getVertexBuffer()->getBufferSize());
	T_ASSERT (singleInstanceMesh->getIndexBuffer()->getIndexType() == render::ItUInt16);

	render::IndexType indexType = vertexCount * InstanceMesh::MaxInstanceCount > 65535 ? render::ItUInt32 : render::ItUInt16;

	// Create instancing mesh.
	Ref< render::Mesh > renderMesh = meshFactory->createMesh(
		singleInstanceMesh->getVertexElements(),
		singleInstanceMesh->getVertexBuffer()->getBufferSize() * InstanceMesh::MaxInstanceCount,
		indexType,
		singleInstanceMesh->getIndexBuffer()->getBufferSize() * InstanceMesh::MaxInstanceCount * (indexType == render::ItUInt16 ? 1 : 2)
	);
	if (!renderMesh)
	{
		log::error << L"Instance mesh create failed; unable to create render mesh" << Endl;
		return 0;
	}

	// Fill instancing vertex buffer.
	uint8_t* destinationVertex = static_cast< uint8_t* >(renderMesh->getVertexBuffer()->lock());
	for (uint32_t i = 0; i < InstanceMesh::MaxInstanceCount; ++i)
	{
		const uint8_t* sourceVertex = static_cast< const uint8_t* >(singleInstanceMesh->getVertexBuffer()->lock());
		for (uint32_t j = 0; j < singleInstanceMesh->getVertexBuffer()->getBufferSize(); j += vertexSize)
		{
			T_ASSERT (j / vertexSize < vertexCount);

			// Copy vertex, patch instance index.
			std::memcpy(destinationVertex, sourceVertex, vertexSize);
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

	if (m_parts.size() != renderMesh->getParts().size())
	{
		log::error << L"Instance mesh create failed; parts mismatch" << Endl;
		return 0;
	}

	Ref< InstanceMesh > instanceMesh = new InstanceMesh();
	instanceMesh->m_mesh = renderMesh;
	instanceMesh->m_parts.resize(m_parts.size());

	for (size_t i = 0; i < m_parts.size(); ++i)
	{
		instanceMesh->m_parts[i].material = m_parts[i].material;
		instanceMesh->m_parts[i].opaque = m_parts[i].opaque;
		if (!resourceManager->bind(instanceMesh->m_parts[i].material))
			return 0;
	}

	return instanceMesh;
}

bool InstanceMeshResource::serialize(ISerializer& s)
{
	return s >> MemberStlVector< Part, MemberComposite< Part > >(L"parts", m_parts);
}

InstanceMeshResource::Part::Part()
:	opaque(true)
{
}

bool InstanceMeshResource::Part::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> Member< Guid >(L"material", material);
	if (s.getVersion() >= 1)
		s >> Member< bool >(L"opaque", opaque);
	return true;
}

	}
}
