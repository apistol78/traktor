#include "Mesh/Static/StaticMeshFactory.h"
#include "Mesh/Static/StaticMeshResource.h"
#include "Mesh/Static/StaticMesh.h"
#include "Resource/IResourceManager.h"
#include "Render/Mesh/RenderMeshFactory.h"
#include "Render/Mesh/MeshReader.h"
#include "Render/Mesh/Mesh.h"
#include "Render/ITexture.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.StaticMeshFactory", StaticMeshFactory, resource::IResourceFactory)

StaticMeshFactory::StaticMeshFactory(db::Database* database, render::IRenderSystem* renderSystem, render::MeshFactory* meshFactory)
:	m_database(database)
,	m_renderSystem(renderSystem)
,	m_meshFactory(meshFactory)
{
	if (!m_meshFactory)
		m_meshFactory = new render::RenderMeshFactory(m_renderSystem);
}

const TypeInfoSet StaticMeshFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< StaticMesh >());
	return typeSet;
}

bool StaticMeshFactory::isCacheable() const
{
	return true;
}

Ref< Object > StaticMeshFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid)
{
	Ref< db::Instance > instance = m_database->getInstance(guid);
	if (!instance)
	{
		log::error << L"Static mesh factory failed; no such instance" << Endl;
		return 0;
	}

	Ref< StaticMeshResource > resource = instance->getObject< StaticMeshResource >();
	if (!resource)
	{
		log::error << L"Static mesh factory failed; unable to checkout resource" << Endl;
		return 0;
	}

	Ref< IStream > dataStream = instance->readData(L"Data");
	if (!dataStream)
	{
		log::error << L"Static mesh factory failed; unable to open data stream" << Endl;
		return 0;
	}

	Ref< render::Mesh > mesh = render::MeshReader(m_meshFactory).read(dataStream);
	if (!mesh)
	{
		log::error << L"Static mesh factory failed; unable to read mesh" << Endl;
		return 0;
	}
	
	dataStream->close();

	const std::vector< StaticMeshResource::Part >& parts = resource->getParts();
	if (parts.size() != mesh->getParts().size())
	{
		log::error << L"Static mesh factory failed; parts mismatch" << Endl;
		return 0;
	}

	Ref< StaticMesh > staticMesh = new StaticMesh();
	staticMesh->m_mesh = mesh;
	staticMesh->m_parts.resize(parts.size());

	for (size_t i = 0; i < parts.size(); ++i)
	{
		staticMesh->m_parts[i].material = parts[i].material;
		if (!resourceManager->bind(staticMesh->m_parts[i].material))
			return 0;
	}

	return staticMesh;
}

	}
}
