#include "Mesh/Skinned/SkinnedMeshFactory.h"
#include "Mesh/Skinned/SkinnedMeshResource.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Resource/IResourceManager.h"
#include "Render/Mesh/RenderMeshFactory.h"
#include "Render/Mesh/MeshReader.h"
#include "Render/Mesh/Mesh.h"
#include "Render/ITexture.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Core/Io/Stream.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.SkinnedMeshFactory", SkinnedMeshFactory, resource::IResourceFactory)

SkinnedMeshFactory::SkinnedMeshFactory(db::Database* database, render::IRenderSystem* renderSystem)
:	m_database(database)
,	m_renderSystem(renderSystem)
{
}

const TypeSet SkinnedMeshFactory::getResourceTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< SkinnedMesh >());
	return typeSet;
}

bool SkinnedMeshFactory::isCacheable() const
{
	return true;
}

Object* SkinnedMeshFactory::create(resource::IResourceManager* resourceManager, const Type& resourceType, const Guid& guid)
{
	Ref< db::Instance > instance = m_database->getInstance(guid);
	if (!instance)
	{
		log::error << L"Skinned mesh factory failed; no such instance" << Endl;
		return 0;
	}

	Ref< SkinnedMeshResource > resource = instance->getObject< SkinnedMeshResource >();
	if (!resource)
	{
		log::error << L"Skinned mesh factory failed; unable to checkout resource" << Endl;
		return 0;
	}

	Ref< Stream > dataStream = instance->readData(L"Data");
	if (!dataStream)
	{
		log::error << L"Skinned mesh factory failed; unable to open data stream" << Endl;
		return 0;
	}

	render::RenderMeshFactory meshFactory(m_renderSystem);
	Ref< render::Mesh > mesh = render::MeshReader(&meshFactory).read(dataStream);
	if (!mesh)
	{
		log::error << L"Skinned mesh factory failed; unable to read mesh" << Endl;
		return 0;
	}
	
	dataStream->close();

	const std::vector< SkinnedMeshResource::Part >& parts = resource->getParts();
	if (parts.size() != mesh->getParts().size())
	{
		log::error << L"Skinned mesh factory failed; parts mismatch" << Endl;
		return 0;
	}

	Ref< SkinnedMesh > skinnedMesh = gc_new< SkinnedMesh >();
	skinnedMesh->m_mesh = mesh;
	skinnedMesh->m_parts.resize(parts.size());

	for (size_t i = 0; i < parts.size(); ++i)
	{
		skinnedMesh->m_parts[i].material = parts[i].material;
		if (!resourceManager->bind(skinnedMesh->m_parts[i].material))
			return 0;
	}

	skinnedMesh->m_boneMap = resource->getBoneMap();

	return skinnedMesh;
}

	}
}
