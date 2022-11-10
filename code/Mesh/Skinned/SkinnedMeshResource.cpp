#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Core/Serialization/MemberStl.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Mesh/Skinned/SkinnedMeshResource.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshReader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.SkinnedMeshResource", 5, SkinnedMeshResource, MeshResource)

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
		return nullptr;
	}

	Ref< SkinnedMesh > skinnedMesh = new SkinnedMesh();

	if (!resourceManager->bind(m_shader, skinnedMesh->m_shader))
		return nullptr;

	skinnedMesh->m_mesh = mesh;

	for (auto i = m_parts.begin(); i != m_parts.end(); ++i)
	{
		render::handle_t worldTechnique = render::getParameterHandle(i->first);
		for (const auto& part : i->second)
		{
			SkinnedMesh::Part sp;
			sp.shaderTechnique = render::getParameterHandle(part.shaderTechnique);
			sp.meshPart = part.meshPart;
			skinnedMesh->m_parts[worldTechnique].push_back(sp);
		}
	}

	int32_t jointMaxIndex = -1;
	for (auto i = m_jointMap.begin(); i != m_jointMap.end(); ++i)
		jointMaxIndex = max< int32_t >(jointMaxIndex, i->second);

	skinnedMesh->m_jointMap = m_jointMap;
	skinnedMesh->m_jointCount = jointMaxIndex + 1;

#if defined(_DEBUG)
	skinnedMesh->m_name = wstombs(name);
#endif

	return skinnedMesh;
}

void SkinnedMeshResource::serialize(ISerializer& s)
{
	T_ASSERT_M(s.getVersion() >= 5, L"Incorrect version");

	MeshResource::serialize(s);

	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> MemberSmallMap<
		std::wstring,
		parts_t,
		Member< std::wstring >,
		MemberStlList< Part, MemberComposite< Part > >
	>(L"parts", m_parts);
	s >> MemberSmallMap< std::wstring, int32_t >(L"jointMap", m_jointMap);
}

void SkinnedMeshResource::Part::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"shaderTechnique", shaderTechnique);
	s >> Member< uint32_t >(L"meshPart", meshPart);
}

}
