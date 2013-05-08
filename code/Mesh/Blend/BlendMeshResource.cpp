#include "Core/RefArray.h"
#include "Core/Io/Reader.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"
#include "Mesh/Blend/BlendMesh.h"
#include "Mesh/Blend/BlendMeshResource.h"
#include "Render/VertexBuffer.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshReader.h"
#include "Render/Mesh/RenderMeshFactory.h"
#include "Render/Mesh/SystemMeshFactory.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.BlendMeshResource", 3, BlendMeshResource, IMeshResource)

Ref< IMesh > BlendMeshResource::createMesh(
	const std::wstring& name,
	IStream* dataStream,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	render::MeshFactory* meshFactory
) const
{
	resource::Proxy< render::Shader > shader;
	if (!resourceManager->bind(m_shader, shader))
		return 0;

	Reader reader(dataStream);

	uint32_t meshCount;
	reader >> meshCount;

	if (!meshCount)
	{
		log::error << L"Blend mesh create failed; no meshes" << Endl;
		return 0;
	}

	render::SystemMeshFactory systemMeshFactory;
	RefArray< render::Mesh > meshes(meshCount);
	std::vector< const uint8_t* > meshVertices(meshCount);

	for (uint32_t i = 0; i < meshCount; ++i)
	{
		if (!(meshes[i] = render::MeshReader(&systemMeshFactory).read(dataStream)))
		{
			log::error << L"Blend mesh create failed; unable to read mesh" << Endl;
			return 0;
		}
		meshVertices[i] = static_cast< const uint8_t* >(meshes[i]->getVertexBuffer()->lock());
		if (!meshVertices[i])
		{
			log::error << L"Blend mesh create failed; unable to lock vertices" << Endl;
			return 0;
		}
	}

	Ref< BlendMesh > blendMesh = new BlendMesh();
	blendMesh->m_renderSystem = renderSystem;
	blendMesh->m_shader = shader;
	blendMesh->m_meshes = meshes;
	blendMesh->m_vertices = meshVertices;

	for (std::map< std::wstring, parts_t >::const_iterator i = m_parts.begin(); i != m_parts.end(); ++i)
	{
		render::handle_t worldTechnique = render::getParameterHandle(i->first);

		for (parts_t::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			BlendMesh::Part part;
			part.shaderTechnique = render::getParameterHandle(j->shaderTechnique);
			part.meshPart = j->meshPart;
			blendMesh->m_parts[worldTechnique].push_back(part);
		}
	}

	blendMesh->m_targetMap = m_targetMap;

	return blendMesh;
}

void BlendMeshResource::serialize(ISerializer& s)
{
	T_ASSERT_M(s.getVersion() >= 3, L"Incorrect version");
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
	s >> MemberStlMap< std::wstring, int >(L"targetMap", m_targetMap);
}

BlendMeshResource::Part::Part()
:	meshPart(0)
{
}

void BlendMeshResource::Part::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"shaderTechnique", shaderTechnique);
	s >> Member< uint32_t >(L"meshPart", meshPart);
}

	}
}
