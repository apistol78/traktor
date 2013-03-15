#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStl.h"
#include "Mesh/Static/StaticMesh.h"
#include "Mesh/Static/StaticMeshResource.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshReader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "World/SwHiZ/OccluderMesh.h"
#include "World/SwHiZ/OccluderMeshReader.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.StaticMeshResource", 4, StaticMeshResource, IMeshResource)

StaticMeshResource::StaticMeshResource()
:	m_haveOccluderMesh(false)
,	m_haveRenderMesh(false)
{
}

Ref< IMesh > StaticMeshResource::createMesh(
	const std::wstring& name,
	IStream* dataStream,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	render::MeshFactory* meshFactory
) const
{
	Ref< world::OccluderMesh > occluderMesh;
	Ref< render::Mesh > renderMesh;
	
	if (m_haveOccluderMesh)
	{
		occluderMesh = world::OccluderMeshReader().read(dataStream);
		if (!occluderMesh)
		{
			log::error << L"Static mesh create failed; unable to read occluder mesh" << Endl;
			return 0;
		}
	}

	if (m_haveRenderMesh)
	{
		renderMesh = render::MeshReader(meshFactory).read(dataStream);
		if (!renderMesh)
		{
			log::error << L"Static mesh create failed; unable to read render mesh" << Endl;
			return 0;
		}
	}

	Ref< StaticMesh > staticMesh = new StaticMesh();
	
	if (!resourceManager->bind(m_shader, staticMesh->m_shader))
		return 0;

	staticMesh->m_occluderMesh = occluderMesh;
	staticMesh->m_renderMesh = renderMesh;

	for (std::map< std::wstring, parts_t >::const_iterator i = m_parts.begin(); i != m_parts.end(); ++i)
	{
		render::handle_t worldTechnique = render::getParameterHandle(i->first);

		staticMesh->m_parts[worldTechnique].reserve(i->second.size());
		for (parts_t::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			StaticMesh::Part part;
			part.shaderTechnique = render::getParameterHandle(j->shaderTechnique);
			part.meshPart = j->meshPart;
			staticMesh->m_parts[worldTechnique].push_back(part);
		}
	}
	
#if defined(_DEBUG)
	staticMesh->m_name = wstombs(name);
#endif

	return staticMesh;
}

bool StaticMeshResource::serialize(ISerializer& s)
{
	T_ASSERT_M(s.getVersion() >= 4, L"Incorrect version");
	s >> Member< bool >(L"haveOccluderMesh", m_haveOccluderMesh);
	s >> Member< bool >(L"haveRenderMesh", m_haveRenderMesh);
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
	return true;
}

StaticMeshResource::Part::Part()
:	meshPart(0)
{
}

bool StaticMeshResource::Part::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"shaderTechnique", shaderTechnique);
	s >> Member< uint32_t >(L"meshPart", meshPart);
	return true;
}

	}
}
