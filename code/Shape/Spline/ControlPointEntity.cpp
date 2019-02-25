#include "Shape/Spline/ControlPointEntity.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.ControlPointEntity", ControlPointEntity, world::ComponentEntity)

ControlPointEntity::ControlPointEntity(const Transform& transform)
:	world::ComponentEntity(transform)
,	m_dirty(true)
{
}

void ControlPointEntity::setTransform(const Transform& transform)
{
	world::ComponentEntity::setTransform(transform);
	m_dirty = true;
}

Aabb3 ControlPointEntity::getBoundingBox() const
{
	Aabb3 boundingBox(
		Vector4(-1.0f, -1.0f, -1.0f),
		Vector4( 1.0f,  1.0f,  1.0f)
	);
	boundingBox.contain(world::ComponentEntity::getBoundingBox());
	return boundingBox;
}

bool ControlPointEntity::checkDirty()
{
	bool dirty = m_dirty;
	m_dirty = false;
	return dirty;
}

	}
}
