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

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.BlendMeshResource", 1, BlendMeshResource, IMeshResource)

Ref< IMesh > BlendMeshResource::createMesh(
	IStream* dataStream,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	render::MeshFactory* meshFactory
) const
{
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
	blendMesh->m_meshes = meshes;
	blendMesh->m_vertices = meshVertices;
	blendMesh->m_parts.resize(m_parts.size());

	for (size_t i = 0; i < m_parts.size(); ++i)
	{
		blendMesh->m_parts[i].material = m_parts[i].material;
		blendMesh->m_parts[i].opaque = m_parts[i].opaque;
		if (!resourceManager->bind(blendMesh->m_parts[i].material))
			return 0;
	}

	blendMesh->m_targetMap = m_targetMap;

	return blendMesh;
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
