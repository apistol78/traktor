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
#include "Resource/Member.h"
#include "World/SwHiZ/OccluderMeshReader.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.InstanceMeshResource", 4, InstanceMeshResource, IMeshResource)

InstanceMeshResource::InstanceMeshResource()
:	m_haveRenderMesh(false)
,	m_haveOccluderMesh(false)
{
}

Ref< IMesh > InstanceMeshResource::createMesh(
	const std::wstring& name,
	IStream* dataStream,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	render::MeshFactory* meshFactory
) const
{
	Ref< world::OccluderMesh > occluderMesh;
	Ref< render::Mesh > renderMesh;

	if (m_haveOccluderMesh)
	{
		occluderMesh = world::OccluderMeshReader().read(dataStream);
		if (!occluderMesh)
		{
			log::error << L"Instance mesh create failed; unable to read occluder mesh" << Endl;
			return 0;
		}
	}

#if !T_USE_LEGACY_INSTANCING

	if (m_haveRenderMesh)
	{
		renderMesh = render::MeshReader(meshFactory).read(dataStream);
		if (!renderMesh)
		{
			log::error << L"Instance mesh create failed; unable to read mesh" << Endl;
			return 0;
		}
	}

#else

	// Read single instance mesh.
	render::SystemMeshFactory systemMeshFactory;
	Ref< render::Mesh > singleInstanceMesh = render::MeshReader(&systemMeshFactory).read(dataStream);
	if (!singleInstanceMesh)
	{
		log::error << L"Instance mesh create failed; unable to read mesh" << Endl;
		return 0;
	}

	const std::vector< render::Mesh::Part >& singleInstanceParts = singleInstanceMesh->getParts();

	uint32_t vertexSize = render::getVertexSize(singleInstanceMesh->getVertexElements());
	uint32_t vertexCount = singleInstanceMesh->getVertexBuffer()->getBufferSize() / vertexSize;
	uint32_t vertexInstanceOffset = findVertexElement(singleInstanceMesh->getVertexElements(), render::DuCustom, 1)->getOffset();

	T_ASSERT (vertexSize * vertexCount == singleInstanceMesh->getVertexBuffer()->getBufferSize());
	T_ASSERT (singleInstanceMesh->getIndexBuffer()->getIndexType() == render::ItUInt16);

	render::IndexType indexType = vertexCount * InstanceMesh::MaxInstanceCount > 65535 ? render::ItUInt32 : render::ItUInt16;

	// Calculate number of indices required in instancing index buffer.
	uint32_t destinationIndicesCount = 0;
	for (uint32_t i = 0; i < singleInstanceParts.size(); ++i)
	{
		const render::Mesh::Part& singleInstancePart = singleInstanceParts[i];
		destinationIndicesCount += InstanceMesh::MaxInstanceCount * singleInstancePart.primitives.count * 3;
	}

	// Create instancing mesh.
	Ref< render::Mesh > renderMesh = meshFactory->createMesh(
		singleInstanceMesh->getVertexElements(),
		singleInstanceMesh->getVertexBuffer()->getBufferSize() * InstanceMesh::MaxInstanceCount,
		indexType,
		destinationIndicesCount * (indexType == render::ItUInt16 ? 2 : 4)
	);
	if (!renderMesh)
	{
		log::error << L"Instance mesh create failed; unable to create render mesh" << Endl;
		return 0;
	}

	// Fill instancing vertex buffer.
	uint8_t* destinationVertex = static_cast< uint8_t* >(renderMesh->getVertexBuffer()->lock());
	const uint8_t* sourceVertexTop = static_cast< const uint8_t* >(singleInstanceMesh->getVertexBuffer()->lock());
	for (uint32_t i = 0; i < InstanceMesh::MaxInstanceCount; ++i)
	{
		const uint8_t* sourceVertex = sourceVertexTop;
		for (uint32_t j = 0; j < singleInstanceMesh->getVertexBuffer()->getBufferSize(); j += vertexSize)
		{
			T_ASSERT (j / vertexSize < vertexCount);

			// Copy vertex, patch instance index.
			std::memcpy(destinationVertex, sourceVertex, vertexSize);
			*reinterpret_cast< float* >(destinationVertex + vertexInstanceOffset) = float(i);

			destinationVertex += vertexSize;
			sourceVertex += vertexSize;
		}
	}
	singleInstanceMesh->getVertexBuffer()->unlock();
	renderMesh->getVertexBuffer()->unlock();

	// Fill instancing index buffer.
	std::vector< render::Mesh::Part > renderParts(singleInstanceParts.size());
	const uint16_t* sourceIndex = static_cast< const uint16_t* >(singleInstanceMesh->getIndexBuffer()->lock());
	uint32_t destinationIndexSize = (indexType == render::ItUInt16 ? 2 : 4);
	uint8_t* destinationIndex = static_cast< uint8_t* >(renderMesh->getIndexBuffer()->lock());
	uint8_t* destinationIndexTop = destinationIndex;
	uint8_t* destinationIndexEnd = destinationIndexTop + destinationIndicesCount * destinationIndexSize;

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
					T_ASSERT (destinationIndex + 3 * sizeof(uint16_t) <= destinationIndexEnd);
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
					T_ASSERT (destinationIndex + 3 * sizeof(uint32_t) <= destinationIndexEnd);
					reinterpret_cast< uint32_t* >(destinationIndex)[0] = sourceIndex[singleInstancePart.primitives.offset + k * 3 + 0] + vertexOffset;
					reinterpret_cast< uint32_t* >(destinationIndex)[1] = sourceIndex[singleInstancePart.primitives.offset + k * 3 + 1] + vertexOffset;
					reinterpret_cast< uint32_t* >(destinationIndex)[2] = sourceIndex[singleInstancePart.primitives.offset + k * 3 + 2] + vertexOffset;
					destinationIndex += 3 * sizeof(uint32_t);
				}
			}
		}
	}

	renderMesh->getIndexBuffer()->unlock();
	singleInstanceMesh->getIndexBuffer()->unlock();

	renderMesh->setParts(renderParts);
	renderMesh->setBoundingBox(singleInstanceMesh->getBoundingBox());

#endif

	Ref< InstanceMesh > instanceMesh = new InstanceMesh();

	if (!resourceManager->bind(m_shader, instanceMesh->m_shader))
		return 0;

	instanceMesh->m_occluderMesh = occluderMesh;
	instanceMesh->m_renderMesh = renderMesh;

	for (std::map< std::wstring, parts_t >::const_iterator i = m_parts.begin(); i != m_parts.end(); ++i)
	{
		render::handle_t worldTechnique = render::getParameterHandle(i->first);

		instanceMesh->m_parts[worldTechnique].reserve(i->second.size());
		for (parts_t::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			InstanceMesh::Part part;
			part.shaderTechnique = render::getParameterHandle(j->shaderTechnique);
			part.meshPart = j->meshPart;
			instanceMesh->m_parts[worldTechnique].push_back(part);
		}
	}

	return instanceMesh;
}

void InstanceMeshResource::serialize(ISerializer& s)
{
	T_ASSERT_M(s.getVersion() >= 4, L"Incorrect version");
	s >> Member< bool >(L"haveOccluderMesh", m_haveOccluderMesh);
	s >> Member< bool >(L"haveRenderMesh", m_haveRenderMesh);
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> MemberStlMap<
		std::wstring,
		parts_t,
		MemberStlPair<
			std::wstring,
			parts_t,
			Member< std::wstring >,
			MemberStlList< Part, MemberComposite< Part > >
		>
	>(L"parts", m_parts);
}

InstanceMeshResource::Part::Part()
:	meshPart(0)
{
}

void InstanceMeshResource::Part::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"shaderTechnique", shaderTechnique);
	s >> Member< uint32_t >(L"meshPart", meshPart);
}

	}
}
