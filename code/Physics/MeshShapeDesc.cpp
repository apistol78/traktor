#include "Core/Serialization/ISerializer.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/Mesh.h"
#include "Physics/MeshResource.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.MeshShapeDesc", 0, MeshShapeDesc, ShapeDesc)

void MeshShapeDesc::setMesh(const resource::Proxy< Mesh >& mesh)
{
	m_mesh = mesh;
}

const resource::Proxy< Mesh >& MeshShapeDesc::getMesh() const
{
	return m_mesh;
}

bool MeshShapeDesc::bind(resource::IResourceManager* resourceManager)
{
	return resourceManager->bind(m_mesh);
}

bool MeshShapeDesc::serialize(ISerializer& s)
{
	if (!ShapeDesc::serialize(s))
		return false;

	return s >> resource::Member< Mesh, MeshResource >(L"mesh", m_mesh);
}

	}
}
