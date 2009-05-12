#include "Mesh/Instance/InstanceMeshResource.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.mesh.InstanceMeshResource", InstanceMeshResource, MeshResource)

void InstanceMeshResource::setParts(const std::vector< Part >& parts)
{
	m_parts = parts;
}

const std::vector< InstanceMeshResource::Part >& InstanceMeshResource::getParts() const
{
	return m_parts;
}

bool InstanceMeshResource::serialize(Serializer& s)
{
	return s >> MemberStlVector< Part, MemberComposite< Part > >(L"parts", m_parts);
}

bool InstanceMeshResource::Part::serialize(Serializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> Member< Guid >(L"material", material);
	return true;
}

	}
}
