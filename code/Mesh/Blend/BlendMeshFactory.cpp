#include "Mesh/Blend/BlendMeshFactory.h"
#include "Mesh/Blend/BlendMeshResource.h"
#include "Mesh/Blend/BlendMesh.h"
#include "Resource/IResourceManager.h"
#include "Render/Mesh/RenderMeshFactory.h"
#include "Render/Mesh/SystemMeshFactory.h"
#include "Render/Mesh/MeshReader.h"
#include "Render/Mesh/Mesh.h"
#include "Render/IRenderSystem.h"
#include "Render/VertexBuffer.h"
#include "Render/IndexBuffer.h"
#include "Render/ITexture.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Core/Io/Stream.h"
#include "Core/Io/Reader.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.BlendMeshFactory", BlendMeshFactory, resource::IResourceFactory)

BlendMeshFactory::BlendMeshFactory(db::Database* db, render::IRenderSystem* renderSystem, render::MeshFactory* meshFactory)
:	m_db(db)
,	m_renderSystem(renderSystem)
,	m_meshFactory(meshFactory)
{
	if (!m_meshFactory)
		m_meshFactory = gc_new< render::RenderMeshFactory >(m_renderSystem);
}

const TypeSet BlendMeshFactory::getResourceTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< BlendMesh >());
	return typeSet;
}

bool BlendMeshFactory::isCacheable() const
{
	return true;
}

Object* BlendMeshFactory::create(resource::IResourceManager* resourceManager, const Type& resourceType, const Guid& guid)
{
	Ref< db::Instance > instance = m_db->getInstance(guid);
	if (!instance)
	{
		log::error << L"Blend mesh factory failed; no such instance" << Endl;
		return 0;
	}

	Ref< BlendMeshResource > resource = instance->getObject< BlendMeshResource >();
	if (!resource)
	{
		log::error << L"Blend mesh factory failed; unable to checkout resource" << Endl;
		return 0;
	}

	Ref< Stream > dataStream = instance->readData(L"Data");
	if (!dataStream)
	{
		log::error << L"Blend mesh factory failed; unable to open data stream" << Endl;
		return 0;
	}

	Reader reader(dataStream);

	uint32_t meshCount;
	reader >> meshCount;

	if (!meshCount)
	{
		log::error << L"Blend mesh factory failed; no meshes" << Endl;
		return 0;
	}

	render::SystemMeshFactory systemMeshFactory;
	RefArray< render::Mesh > meshes(meshCount);
	std::vector< const uint8_t* > meshVertices(meshCount);

	for (uint32_t i = 0; i < meshCount; ++i)
	{
		if (!(meshes[i] = render::MeshReader(&systemMeshFactory).read(dataStream)))
		{
			log::error << L"Blend mesh factory failed; unable to read mesh" << Endl;
			return 0;
		}
		meshVertices[i] = static_cast< const uint8_t* >(meshes[i]->getVertexBuffer()->lock());
		if (!meshVertices[i])
		{
			log::error << L"Blend mesh factory failed; unable to lock vertices" << Endl;
			return 0;
		}
	}
	
	dataStream->close();

	// Attach materials to parts.
	const std::vector< BlendMeshResource::Part >& parts = resource->getParts();

	Ref< BlendMesh > blendMesh = gc_new< BlendMesh >();
	blendMesh->m_renderSystem = m_renderSystem;
	blendMesh->m_meshes = meshes;
	blendMesh->m_vertices = meshVertices;
	blendMesh->m_parts.resize(parts.size());

	for (size_t i = 0; i < parts.size(); ++i)
	{
		blendMesh->m_parts[i].material = parts[i].material;
		if (!resourceManager->bind(blendMesh->m_parts[i].material))
			return 0;
	}

	blendMesh->m_targetMap = resource->getBlendTargetMap();

	return blendMesh;
}

	}
}
