#include "Mesh/MeshEntity.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.MeshEntity", MeshEntity, world::Entity)

MeshEntity::MeshEntity(const Transform& transform)
:	m_transform(transform)
,	m_userParameter(0.0f)
,	m_parameterCallback(0)
{
}

void MeshEntity::update(const UpdateParams& update)
{
	m_transform.step();
}

void MeshEntity::setTransform(const Transform& transform)
{
	m_transform.set(transform);
}

bool MeshEntity::getTransform(Transform& outTransform) const
{
	outTransform = m_transform.get();
	return true;
}

Transform MeshEntity::getTransform(float interval) const
{
	return m_transform.get(interval);
}

	}
}
