#include "Mesh/MeshEntity.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.MeshEntity", MeshEntity, world::SpatialEntity)

MeshEntity::MeshEntity(const Transform& transform)
:	m_transform(transform)
,	m_transformPrevious(transform)
,	m_userParameter(0.0f)
,	m_parameterCallback(0)
,	m_first(true)
{
}

void MeshEntity::update(const world::EntityUpdate* update)
{
	m_transformPrevious = m_transform;
}

void MeshEntity::setTransform(const Transform& transform)
{
	m_transform = transform;
	if (m_first)
		m_transformPrevious = transform;
}

bool MeshEntity::getTransform(Transform& outTransform) const
{
	outTransform = m_transform;
	return true;
}

Transform MeshEntity::getTransform(float interval) const
{
	return lerp(m_transformPrevious, m_transform, Scalar(interval));
}

	}
}
