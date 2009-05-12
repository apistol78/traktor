#include "Mesh/MeshEntity.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.MeshEntity", MeshEntity, world::SpatialEntity)

MeshEntity::MeshEntity(const Matrix44& transform)
:	m_transform(transform)
,	m_parameterCallback(0)
{
}

void MeshEntity::setTransform(const Matrix44& transform)
{
	m_transform = transform;
}

bool MeshEntity::getTransform(Matrix44& outTransform) const
{
	outTransform = m_transform;
	return true;
}

	}
}
