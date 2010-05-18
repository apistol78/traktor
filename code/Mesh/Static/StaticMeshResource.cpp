#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStl.h"
#include "Mesh/Static/StaticMesh.h"
#include "Mesh/Static/StaticMeshResource.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshReader.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.StaticMeshResource", 2, StaticMeshResource, IMeshResource)

Ref< IMesh > StaticMeshResource::createMesh(
	IStream* dataStream,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	render::MeshFactory* meshFactory
) const
{
	Ref< render::Mesh > mesh = render::MeshReader(meshFactory).read(dataStream);
	if (!mesh)
	{
		log::error << L"Static mesh create failed; unable to read mesh" << Endl;
		return 0;
	}

	Ref< StaticMesh > staticMesh = new StaticMesh();
	staticMesh->m_shader = m_shader;
	staticMesh->m_mesh = mesh;

	for (std::map< std::wstring, parts_t >::const_iterator i = m_parts.begin(); i != m_parts.end(); ++i)
	{
		render::handle_t worldTechnique = render::getParameterHandle(i->first);

		for (parts_t::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			StaticMesh::Part part;
			part.shaderTechnique = render::getParameterHandle(j->shaderTechnique);
			part.meshPart = j->meshPart;
			part.opaque = j->opaque;
			staticMesh->m_parts[worldTechnique].push_back(part);
		}
	}

	if (!resourceManager->bind(staticMesh->m_shader))
		return 0;

	return staticMesh;
}

bool StaticMeshResource::serialize(ISerializer& s)
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
	return true;
}

StaticMeshResource::Part::Part()
:	meshPart(0)
,	opaque(true)
{
}

bool StaticMeshResource::Part::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"shaderTechnique", shaderTechnique);
	s >> Member< uint32_t >(L"meshPart", meshPart);
	s >> Member< bool >(L"opaque", opaque);
	return true;
}

	}
}
