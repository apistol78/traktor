#include "Mesh/Skinned/SkinnedMeshResource.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.SkinnedMeshResource", 1, SkinnedMeshResource, MeshResource)

void SkinnedMeshResource::setParts(const std::vector< Part >& parts)
{
	m_parts = parts;
}

const std::vector< SkinnedMeshResource::Part >& SkinnedMeshResource::getParts() const
{
	return m_parts;
}

void SkinnedMeshResource::setBone(const std::wstring& boneName, int boneIndex)
{
	m_boneMap[boneName] = boneIndex;
}

const std::map< std::wstring, int >& SkinnedMeshResource::getBoneMap() const
{
	return m_boneMap;
}

bool SkinnedMeshResource::serialize(ISerializer& s)
{
	s >> MemberStlVector< Part, MemberComposite< Part > >(L"parts", m_parts);
	s >> MemberStlMap< std::wstring, int >(L"boneMap", m_boneMap);
	return true;
}

SkinnedMeshResource::Part::Part()
:	opaque(true)
{
}

bool SkinnedMeshResource::Part::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> Member< Guid >(L"material", material);
	if (s.getVersion() >= 1)
		s >> Member< bool >(L"opaque", opaque);
	return true;
}

	}
}
