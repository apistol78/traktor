#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Mesh/Skinned/SkinnedMeshResource.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshReader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.SkinnedMeshResource", 4, SkinnedMeshResource, IMeshResource)

Ref< IMesh > SkinnedMeshResource::createMesh(
	const std::wstring& name,
	IStream* dataStream,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	render::MeshFactory* meshFactory
) const
{
	Ref< render::Mesh > mesh = render::MeshReader(meshFactory).read(dataStream);
	if (!mesh)
	{
		log::error << L"Skinned mesh create failed; unable to read mesh" << Endl;
		return 0;
	}

	Ref< SkinnedMesh > skinnedMesh = new SkinnedMesh();

	if (!resourceManager->bind(m_shader, skinnedMesh->m_shader))
		return 0;

	skinnedMesh->m_mesh = mesh;

	for (std::map< std::wstring, parts_t >::const_iterator i = m_parts.begin(); i != m_parts.end(); ++i)
	{
		render::handle_t worldTechnique = render::getParameterHandle(i->first);

		for (parts_t::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			SkinnedMesh::Part part;
			part.shaderTechnique = render::getParameterHandle(j->shaderTechnique);
			part.meshPart = j->meshPart;
			skinnedMesh->m_parts[worldTechnique].push_back(part);
		}
	}

	int32_t jointMaxIndex = -1;
	for (std::map< std::wstring, int >::const_iterator i = m_jointMap.begin(); i != m_jointMap.end(); ++i)
		jointMaxIndex = max< int32_t >(jointMaxIndex, i->second);

	skinnedMesh->m_jointMap = m_jointMap;
	skinnedMesh->m_jointCount = jointMaxIndex + 1;

	return skinnedMesh;
}

bool SkinnedMeshResource::serialize(ISerializer& s)
{
	T_ASSERT_M(s.getVersion() >= 4, L"Incorrect version");
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> MemberStlMap<
		std::wstring,
		parts_t,
		MemberStlPair<
			std::wstring,
			parts_t,
			Member< std::wstring >,
			MemberStlList< Part, MemberComposite< Part > >
		>
	>(L"parts", m_parts);
	s >> MemberStlMap< std::wstring, int32_t >(L"jointMap", m_jointMap);
	return true;
}

SkinnedMeshResource::Part::Part()
:	meshPart(0)
{
}

bool SkinnedMeshResource::Part::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"shaderTechnique", shaderTechnique);
	s >> Member< uint32_t >(L"meshPart", meshPart);
	return true;
}

	}
}
