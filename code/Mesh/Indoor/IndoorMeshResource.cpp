#include "Mesh/Indoor/IndoorMeshResource.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.IndoorMeshResource", 1, IndoorMeshResource, MeshResource)

void IndoorMeshResource::setSectors(const AlignedVector< Sector >& sectors)
{
	m_sectors = sectors;
}

const AlignedVector< IndoorMeshResource::Sector >& IndoorMeshResource::getSectors() const
{
	return m_sectors;
}

void IndoorMeshResource::setPortals(const AlignedVector< Portal >& portals)
{
	m_portals = portals;
}

const AlignedVector< IndoorMeshResource::Portal >& IndoorMeshResource::getPortals() const
{
	return m_portals;
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
