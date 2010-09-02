#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Mesh/Skinned/SkinnedMeshResource.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshReader.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.SkinnedMeshResource", 2, SkinnedMeshResource, IMeshResource)

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
	skinnedMesh->m_shader = m_shader;
	skinnedMesh->m_mesh = mesh;

	for (std::map< std::wstring, parts_t >::const_iterator i = m_parts.begin(); i != m_parts.end(); ++i)
	{
		render::handle_t worldTechnique = render::getParameterHandle(i->first);

		for (parts_t::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			SkinnedMesh::Part part;
			part.shaderTechnique = render::getParameterHandle(j->shaderTechnique);
			part.meshPart = j->meshPart;
			part.opaque = j->opaque;
			skinnedMesh->m_parts[worldTechnique].push_back(part);
		}
	}

	int32_t boneMaxIndex = -1;
	for (std::map< std::wstring, int >::const_iterator i = m_boneMap.begin(); i != m_boneMap.end(); ++i)
		boneMaxIndex = max< int32_t >(boneMaxIndex, i->second);

	skinnedMesh->m_boneMap = m_boneMap;
	skinnedMesh->m_boneCount = boneMaxIndex + 1;

	if (!resourceManager->bind(skinnedMesh->m_shader))
		return 0;

	return skinnedMesh;
}

bool SkinnedMeshResource::serialize(ISerializer& s)
{
	T_ASSERT_M(s.getVersion() >= 2, L"Incorrect version");
	s >> Member< Guid >(L"shader", m_shader);
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
	s >> MemberStlMap< std::wstring, int >(L"boneMap", m_boneMap);
	return true;
}

SkinnedMeshResource::Part::Part()
:	meshPart(0)
,	opaque(true)
{
}

bool SkinnedMeshResource::Part::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"shaderTechnique", shaderTechnique);
	s >> Member< uint32_t >(L"meshPart", meshPart);
	s >> Member< bool >(L"opaque", opaque);
	return true;
}

	}
}
