#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Mesh/IMesh.h"
#include "Mesh/IMeshResource.h"
#include "Mesh/MeshFactory.h"
#include "Mesh/Blend/BlendMesh.h"
#include "Mesh/Indoor/IndoorMesh.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Mesh/Static/StaticMesh.h"
#include "Mesh/Stream/StreamMesh.h"
#include "Render/Mesh/RenderMeshFactory.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.MeshFactory", MeshFactory, resource::IResourceFactory)

MeshFactory::MeshFactory(db::Database* database, render::IRenderSystem* renderSystem, render::MeshFactory* meshFactory)
:	m_database(database)
,	m_renderSystem(renderSystem)
,	m_meshFactory(meshFactory)
{
	if (!m_meshFactory)
		m_meshFactory = new render::RenderMeshFactory(m_renderSystem);
}

const TypeInfoSet MeshFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< IMesh >());
	typeSet.insert(&type_of< BlendMesh >());
	typeSet.insert(&type_of< IndoorMesh >());
	typeSet.insert(&type_of< InstanceMesh >());
	typeSet.insert(&type_of< SkinnedMesh >());
	typeSet.insert(&type_of< StaticMesh >());
	typeSet.insert(&type_of< StreamMesh >());
	return typeSet;
}

bool MeshFactory::isCacheable() const
{
	return true;
}

Ref< Object > MeshFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid)
{
	Ref< db::Instance > instance = m_database->getInstance(guid);
	if (!instance)
	{
		log::error << L"Mesh factory failed; no such instance" << Endl;
		return 0;
	}

	Ref< IMeshResource > resource = instance->getObject< IMeshResource >();
	if (!resource)
	{
		log::error << L"Mesh factory failed; unable to checkout resource" << Endl;
		return 0;
	}

	Ref< IStream > dataStream = instance->readData(L"Data");
	if (!dataStream)
	{
		log::error << L"Mesh factory failed; unable to open data stream" << Endl;
		return 0;
	}

	Ref< IMesh > mesh = resource->createMesh(instance->getPath(), dataStream, resourceManager, m_renderSystem, m_meshFactory);
	if (!mesh)
	{
		log::error << L"Mesh factory failed; unable to create mesh" << Endl;
		return 0;
	}

	if (!is_type_of(resourceType, type_of(mesh)))
	{
		log::error << L"Mesh factory failed; incorrect type of mesh, must be a " << resourceType.getName() << Endl;
		return 0;
	}
	
	return mesh;
}

	}
}
