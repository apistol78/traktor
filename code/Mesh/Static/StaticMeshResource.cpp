#include "Mesh/Static/StaticMeshResource.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.StaticMeshResource", 1, StaticMeshResource, MeshResource)

void StaticMeshResource::setParts(const std::vector< Part >& parts)
{
	m_parts = parts;
}

const std::vector< StaticMeshResource::Part >& StaticMeshResource::getParts() const
{
	return m_parts;
}

bool StaticMeshResource::serialize(ISerializer& s)
{
	return s >> MemberStlVector< Part, MemberComposite< Part > >(L"parts", m_parts);
}

StaticMeshResource::Part::Part()
:	opaque(true)
{
}

bool StaticMeshResource::Part::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> Member< Guid >(L"material", material);
	if (s.getVersion() >= 1)
		s >> Member< bool >(L"opaque", opaque);
	return true;
}

	}
}
