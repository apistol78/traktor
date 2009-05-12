#include "Mesh/Skinned/SkinnedMeshResource.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.mesh.SkinnedMeshResource", SkinnedMeshResource, MeshResource)

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

bool SkinnedMeshResource::serialize(Serializer& s)
{
	s >> MemberStlVector< Part, MemberComposite< Part > >(L"parts", m_parts);
	s >> MemberStlMap< std::wstring, int >(L"boneMap", m_boneMap);
	return true;
}

bool SkinnedMeshResource::Part::serialize(Serializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> Member< Guid >(L"material", material);
	return true;
}

	}
}
