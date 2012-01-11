#include "Mesh/MeshEntity.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.MeshEntity", MeshEntity, world::SpatialEntity)

MeshEntity::MeshEntity(const Transform& transform)
:	m_userParameter(0.0f)
,	m_parameterCallback(0)
{
	m_transform[0] =
	m_transform[1] = transform;
}

void MeshEntity::update(const world::EntityUpdate* update)
{
}

void MeshEntity::setTransform(const Transform& transform)
{
	m_transform[0] = m_transform[1];
	m_transform[1] = transform;
}

bool MeshEntity::getTransform(Transform& outTransform) const
{
	outTransform = m_transform[1];
	return true;
}

Transform MeshEntity::getTransform(float interval) const
{
	return lerp(m_transform[0], m_transform[1], Scalar(interval));
}

	}
}
