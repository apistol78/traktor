#include "Mesh/MeshEntity.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.MeshEntity", MeshEntity, world::Entity)

MeshEntity::MeshEntity(const Transform& transform, bool screenSpaceCulling)
:	m_transform(transform)
,	m_parameterCallback(0)
,	m_screenSpaceCulling(screenSpaceCulling)
{
}

void MeshEntity::update(const world::UpdateParams& update)
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
