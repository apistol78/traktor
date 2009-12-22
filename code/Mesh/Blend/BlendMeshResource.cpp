#include "Mesh/Blend/BlendMeshResource.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.BlendMeshResource", 1, BlendMeshResource, MeshResource)

void BlendMeshResource::setParts(const std::vector< Part >& parts)
{
	m_parts = parts;
}

const std::vector< BlendMeshResource::Part >& BlendMeshResource::getParts() const
{
	return m_parts;
}

void BlendMeshResource::setBlendTarget(const std::wstring& name, int index)
{
	m_targetMap[name] = index;
}

const std::map< std::wstring, int >& BlendMeshResource::getBlendTargetMap() const
{
	return m_targetMap;
}

bool BlendMeshResource::serialize(ISerializer& s)
{
	s >> MemberStlVector< Part, MemberComposite< Part > >(L"parts", m_parts);
	s >> MemberStlMap< std::wstring, int >(L"targetMap", m_targetMap);
	return true;
}

BlendMeshResource::Part::Part()
:	opaque(true)
{
}

bool BlendMeshResource::Part::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> Member< Guid >(L"material", material);
	if (s.getVersion() >= 1)
		s >> Member< bool >(L"opaque", opaque);
	return true;
}

	}
}
