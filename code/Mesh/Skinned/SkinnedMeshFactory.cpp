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
#include "Core/Io/IStream.h"
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

const TypeInfoSet SkinnedMeshFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< SkinnedMesh >());
	return typeSet;
}

bool SkinnedMeshFactory::isCacheable() const
{
	return true;
}

Ref< Object > SkinnedMeshFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid)
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

	Ref< IStream > dataStream = instance->readData(L"Data");
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

	Ref< SkinnedMesh > skinnedMesh = new SkinnedMesh();
	skinnedMesh->m_mesh = mesh;
	skinnedMesh->m_parts.resize(parts.size());

	for (size_t i = 0; i < parts.size(); ++i)
	{
		skinnedMesh->m_parts[i].material = parts[i].material;
		skinnedMesh->m_parts[i].opaque = parts[i].opaque;
		if (!resourceManager->bind(skinnedMesh->m_parts[i].material))
			return 0;
	}

	const std::map< std::wstring, int >& boneMap = resource->getBoneMap();

	int32_t boneMaxIndex = -1;
	for (std::map< std::wstring, int >::const_iterator i = boneMap.begin(); i != boneMap.end(); ++i)
		boneMaxIndex = max< int32_t >(boneMaxIndex, i->second);

	skinnedMesh->m_boneMap = boneMap;
	skinnedMesh->m_boneCount = boneMaxIndex + 1;

	return skinnedMesh;
}

	}
}
