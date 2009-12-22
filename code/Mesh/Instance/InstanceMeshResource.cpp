#include "Mesh/Instance/InstanceMeshResource.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.InstanceMeshResource", 1, InstanceMeshResource, MeshResource)

void InstanceMeshResource::setParts(const std::vector< Part >& parts)
{
	m_parts = parts;
}

const std::vector< InstanceMeshResource::Part >& InstanceMeshResource::getParts() const
{
	return m_parts;
}

bool InstanceMeshResource::serialize(ISerializer& s)
{
	return s >> MemberStlVector< Part, MemberComposite< Part > >(L"parts", m_parts);
}

InstanceMeshResource::Part::Part()
:	opaque(true)
{
}

bool InstanceMeshResource::Part::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> Member< Guid >(L"material", material);
	if (s.getVersion() >= 1)
		s >> Member< bool >(L"opaque", opaque);
	return true;
}

	}
}
