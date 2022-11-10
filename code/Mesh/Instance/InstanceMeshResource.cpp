#include <algorithm>
#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStl.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Mesh/Instance/InstanceMeshResource.h"
#include "Render/Buffer.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshReader.h"
#include "Render/Mesh/SystemMeshFactory.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.InstanceMeshResource", 7, InstanceMeshResource, MeshResource)

Ref< IMesh > InstanceMeshResource::createMesh(
	const std::wstring& name,
	IStream* dataStream,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	render::MeshFactory* meshFactory
) const
{
	Ref< render::Mesh > renderMesh;

	if (m_haveRenderMesh)
	{
		renderMesh = render::MeshReader(meshFactory).read(dataStream);
		if (!renderMesh)
		{
			log::error << L"Instance mesh create failed; unable to read mesh." << Endl;
			return nullptr;
		}
	}

	Ref< InstanceMesh > instanceMesh = new InstanceMesh();

	if (!resourceManager->bind(m_shader, instanceMesh->m_shader))
		return nullptr;

	instanceMesh->m_renderMesh = renderMesh;

	for (std::map< std::wstring, parts_t >::const_iterator i = m_parts.begin(); i != m_parts.end(); ++i)
	{
		render::handle_t worldTechnique = render::getParameterHandle(i->first);

		instanceMesh->m_parts[worldTechnique].reserve(i->second.size());
		for (parts_t::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			InstanceMesh::Part part;
			part.shaderTechnique = render::getParameterHandle(j->shaderTechnique);
			part.meshPart = j->meshPart;
			instanceMesh->m_parts[worldTechnique].push_back(part);
		}
	}

	instanceMesh->m_maxInstanceCount = std::min< int32_t >(m_maxInstanceCount, InstanceMesh::MaxInstanceCount);
	return instanceMesh;
}

void InstanceMeshResource::serialize(ISerializer& s)
{
	T_ASSERT_M(s.getVersion() >= 7, L"Incorrect version");

	MeshResource::serialize(s);

	s >> Member< bool >(L"haveRenderMesh", m_haveRenderMesh);
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> MemberStlMap<
		std::wstring,
		parts_t,
		Member< std::wstring >,
		MemberStlList< Part, MemberComposite< Part > >
	>(L"parts", m_parts);
	s >> Member< int32_t >(L"maxInstanceCount", m_maxInstanceCount);
}

void InstanceMeshResource::Part::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"shaderTechnique", shaderTechnique);
	s >> Member< uint32_t >(L"meshPart", meshPart);
}

}
