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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.StaticMeshResource", 1, StaticMeshResource, IMeshResource)

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

	if (m_parts.size() != mesh->getParts().size())
	{
		log::error << L"Static mesh create failed; parts mismatch" << Endl;
		return 0;
	}

	Ref< StaticMesh > staticMesh = new StaticMesh();
	staticMesh->m_mesh = mesh;
	staticMesh->m_parts.resize(m_parts.size());

	for (size_t i = 0; i < m_parts.size(); ++i)
	{
		staticMesh->m_parts[i].material = m_parts[i].material;
		staticMesh->m_parts[i].opaque = m_parts[i].opaque;
		if (!resourceManager->bind(staticMesh->m_parts[i].material))
			return 0;
	}

	return staticMesh;
}

bool StaticMeshResource::serialize(ISerializer& s)
{
	return s >> MemberStlVector< Part, MemberComposite< Part > >(L"parts", m_parts);
}

StaticMeshResource::Part::Part()
:	opaque(true)
{
}

bool StaticMeshResource::Part::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> Member< Guid >(L"material", material);
	if (s.getVersion() >= 1)
		s >> Member< bool >(L"opaque", opaque);
	return true;
}

	}
}
