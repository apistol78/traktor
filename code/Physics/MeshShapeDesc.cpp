#include "Physics/MeshShapeDesc.h"
#include "Physics/Mesh.h"
#include "Physics/MeshResource.h"
#include "Core/Serialization/ISerializer.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.MeshShapeDesc", MeshShapeDesc, ShapeDesc)

void MeshShapeDesc::setMesh(const resource::Proxy< Mesh >& mesh)
{
	m_mesh = mesh;
}

const resource::Proxy< Mesh >& MeshShapeDesc::getMesh() const
{
	return m_mesh;
}

bool MeshShapeDesc::serialize(ISerializer& s)
{
	if (!ShapeDesc::serialize(s))
		return false;

	return s >> resource::Member< Mesh, MeshResource >(L"mesh", m_mesh);
}

	}
}
