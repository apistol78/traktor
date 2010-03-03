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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.SkinnedMeshResource", 1, SkinnedMeshResource, IMeshResource)

Ref< IMesh > SkinnedMeshResource::createMesh(
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

	if (m_parts.size() != mesh->getParts().size())
	{
		log::error << L"Skinned mesh create failed; parts mismatch" << Endl;
		return 0;
	}

	Ref< SkinnedMesh > skinnedMesh = new SkinnedMesh();
	skinnedMesh->m_mesh = mesh;
	skinnedMesh->m_parts.resize(m_parts.size());

	for (size_t i = 0; i < m_parts.size(); ++i)
	{
		skinnedMesh->m_parts[i].material = m_parts[i].material;
		skinnedMesh->m_parts[i].opaque = m_parts[i].opaque;
		if (!resourceManager->bind(skinnedMesh->m_parts[i].material))
			return 0;
	}

	int32_t boneMaxIndex = -1;
	for (std::map< std::wstring, int >::const_iterator i = m_boneMap.begin(); i != m_boneMap.end(); ++i)
		boneMaxIndex = max< int32_t >(boneMaxIndex, i->second);

	skinnedMesh->m_boneMap = m_boneMap;
	skinnedMesh->m_boneCount = boneMaxIndex + 1;

	return skinnedMesh;
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
