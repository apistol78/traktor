#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStl.h"
#include "Mesh/Indoor/IndoorMesh.h"
#include "Mesh/Indoor/IndoorMeshResource.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshReader.h"
#include "Render/Mesh/RenderMeshFactory.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.IndoorMeshResource", 1, IndoorMeshResource, IMeshResource)

Ref< IMesh > IndoorMeshResource::createMesh(
	IStream* dataStream,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	render::MeshFactory* meshFactory
) const
{
	Ref< render::Mesh > mesh = render::MeshReader(meshFactory).read(dataStream);
	if (!mesh)
	{
		log::error << L"Indoor mesh create failed; unable to read mesh" << Endl;
		return 0;
	}

	Ref< IndoorMesh > indoorMesh = new IndoorMesh();
	indoorMesh->m_mesh = mesh;

	indoorMesh->m_sectors.resize(m_sectors.size());
	for (size_t i = 0; i < m_sectors.size(); ++i)
	{
		indoorMesh->m_sectors[i].boundingBox = Aabb(m_sectors[i].min, m_sectors[i].max);

		const std::vector< IndoorMeshResource::Part >& sectorParts = m_sectors[i].parts;

		indoorMesh->m_sectors[i].parts.resize(sectorParts.size());
		for (size_t j = 0; j < sectorParts.size(); ++j)
		{
			indoorMesh->m_sectors[i].parts[j].material = sectorParts[j].material;
			indoorMesh->m_sectors[i].parts[j].meshPart = sectorParts[j].meshPart;
			indoorMesh->m_sectors[i].parts[i].opaque = sectorParts[j].opaque;

			if (!resourceManager->bind(indoorMesh->m_sectors[i].parts[j].material))
				return 0;
		}
	}

	indoorMesh->m_portals.resize(m_portals.size());
	for (size_t i = 0; i < m_portals.size(); ++i)
	{
		indoorMesh->m_portals[i].winding.points = m_portals[i].pts;
		indoorMesh->m_portals[i].sectorA = m_portals[i].sectorA;
		indoorMesh->m_portals[i].sectorB = m_portals[i].sectorB;
	}

	return indoorMesh;
}

bool IndoorMeshResource::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< Sector, MemberComposite< Sector > >(L"sectors", m_sectors);
	s >> MemberAlignedVector< Portal, MemberComposite< Portal > >(L"portals", m_portals);
	return true;
}

IndoorMeshResource::Part::Part()
:	opaque(true)
{
}

bool IndoorMeshResource::Part::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"material", material);
	s >> Member< int32_t >(L"meshPart", meshPart);
	if (s.getVersion() >= 1)
		s >> Member< bool >(L"opaque", opaque);
	return true;
}

bool IndoorMeshResource::Sector::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"min", min);
	s >> Member< Vector4 >(L"max", max);
	s >> MemberStlVector< Part, MemberComposite< Part > >(L"parts", parts);
	return true;
}

bool IndoorMeshResource::Portal::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< Vector4 >(L"pts", pts);
	s >> Member< int32_t >(L"sectorA", sectorA);
	s >> Member< int32_t >(L"sectorB", sectorB);
	return true;
}

	}
}
