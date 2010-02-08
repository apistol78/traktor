#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Mesh/Stream/StreamMeshFactory.h"
#include "Mesh/Stream/StreamMeshResource.h"
#include "Mesh/Stream/StreamMesh.h"
#include "Render/ITexture.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshReader.h"
#include "Render/Mesh/RenderMeshFactory.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.StreamMeshFactory", StreamMeshFactory, resource::IResourceFactory)

StreamMeshFactory::StreamMeshFactory(db::Database* database, render::IRenderSystem* renderSystem, render::MeshFactory* meshFactory)
:	m_database(database)
,	m_renderSystem(renderSystem)
,	m_meshFactory(meshFactory)
{
	if (!m_meshFactory)
		m_meshFactory = new render::RenderMeshFactory(m_renderSystem);
}

const TypeInfoSet StreamMeshFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< StreamMesh >());
	return typeSet;
}

bool StreamMeshFactory::isCacheable() const
{
	return true;
}

Ref< Object > StreamMeshFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid)
{
	Ref< db::Instance > instance = m_database->getInstance(guid);
	if (!instance)
	{
		log::error << L"Stream mesh factory failed; no such instance" << Endl;
		return 0;
	}

	Ref< StreamMeshResource > resource = instance->getObject< StreamMeshResource >();
	if (!resource)
	{
		log::error << L"Stream mesh factory failed; unable to checkout resource" << Endl;
		return 0;
	}

	Ref< IStream > dataStream = instance->readData(L"Data");
	if (!dataStream)
	{
		log::error << L"Stream mesh factory failed; unable to open data stream" << Endl;
		return 0;
	}

	Ref< StreamMesh > streamMesh = new StreamMesh();
	streamMesh->m_stream = dataStream;
	streamMesh->m_meshReader = new render::MeshReader(m_meshFactory);
	streamMesh->m_frameOffsets = resource->getFrameOffsets();
	streamMesh->m_boundingBox = resource->getBoundingBox();
	
	const std::vector< StreamMeshResource::Part >& parts = resource->getParts();
	for (std::vector< StreamMeshResource::Part >::const_iterator i = parts.begin(); i != parts.end(); ++i)
	{
		StreamMesh::Part part;
		part.material = i->material;
		part.opaque = i->opaque;

		if (resourceManager->bind(part.material))
			streamMesh->m_parts[i->name] = part;
		else
			return 0;
	}

	return streamMesh;
}

	}
}
