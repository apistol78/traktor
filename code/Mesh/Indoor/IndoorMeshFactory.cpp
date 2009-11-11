#include <limits>
#include "Mesh/Indoor/IndoorMeshFactory.h"
#include "Mesh/Indoor/IndoorMeshResource.h"
#include "Mesh/Indoor/IndoorMesh.h"
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.IndoorMeshFactory", IndoorMeshFactory, resource::IResourceFactory)

IndoorMeshFactory::IndoorMeshFactory(db::Database* database, render::IRenderSystem* renderSystem)
:	m_database(database)
,	m_renderSystem(renderSystem)
{
}

const TypeSet IndoorMeshFactory::getResourceTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< IndoorMesh >());
	return typeSet;
}

bool IndoorMeshFactory::isCacheable() const
{
	return true;
}

Ref< Object > IndoorMeshFactory::create(resource::IResourceManager* resourceManager, const Type& resourceType, const Guid& guid)
{
	Ref< db::Instance > instance = m_database->getInstance(guid);
	if (!instance)
	{
		log::error << L"Indoor mesh factory failed; no such instance" << Endl;
		return 0;
	}

	Ref< IndoorMeshResource > resource = instance->getObject< IndoorMeshResource >();
	if (!resource)
	{
		log::error << L"Indoor mesh factory failed; unable to checkout resource" << Endl;
		return 0;
	}

	Ref< Stream > dataStream = instance->readData(L"Data");
	if (!dataStream)
	{
		log::error << L"Indoor mesh factory failed; unable to open data stream" << Endl;
		return 0;
	}

	render::RenderMeshFactory meshFactory(m_renderSystem);
	Ref< render::Mesh > mesh = render::MeshReader(&meshFactory).read(dataStream);
	if (!mesh)
	{
		log::error << L"Indoor mesh factory failed; unable to read mesh" << Endl;
		return 0;
	}
	
	dataStream->close();

	Ref< IndoorMesh > indoorMesh = gc_new< IndoorMesh >();
	indoorMesh->m_mesh = mesh;

	const AlignedVector< IndoorMeshResource::Sector >& sectors = resource->getSectors();

	indoorMesh->m_sectors.resize(sectors.size());
	for (size_t i = 0; i < sectors.size(); ++i)
	{
		indoorMesh->m_sectors[i].boundingBox = Aabb(sectors[i].min, sectors[i].max);

		const std::vector< IndoorMeshResource::Part >& sectorParts = sectors[i].parts;

		indoorMesh->m_sectors[i].parts.resize(sectorParts.size());
		for (size_t j = 0; j < sectorParts.size(); ++j)
		{
			indoorMesh->m_sectors[i].parts[j].material = sectorParts[j].material;
			indoorMesh->m_sectors[i].parts[j].meshPart = sectorParts[j].meshPart;

			if (!resourceManager->bind(indoorMesh->m_sectors[i].parts[j].material))
				return 0;
		}
	}

	const AlignedVector< IndoorMeshResource::Portal >& portals = resource->getPortals();

	indoorMesh->m_portals.resize(portals.size());
	for (size_t i = 0; i < portals.size(); ++i)
	{
		indoorMesh->m_portals[i].winding.points = portals[i].pts;
		indoorMesh->m_portals[i].sectorA = portals[i].sectorA;
		indoorMesh->m_portals[i].sectorB = portals[i].sectorB;
	}

	return indoorMesh;
}

	}
}
