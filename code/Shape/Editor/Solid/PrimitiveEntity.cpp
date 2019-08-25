#include "Shape/Editor/Solid/PrimitiveEntity.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.PrimitiveEntity", PrimitiveEntity, world::Entity)

PrimitiveEntity::PrimitiveEntity(const Transform& transform, BooleanOperation operation)
:   m_transform(transform)
,   m_operation(operation)
,   m_dirty(true)
{
}

void PrimitiveEntity::setTransform(const Transform& transform)
{
    if (m_transform != transform)
    {
        m_transform = transform;
        m_dirty = true;
    }
}

 bool PrimitiveEntity::getTransform(Transform& outTransform) const
 {
     outTransform = m_transform;
     return true;
 }

 Aabb3 PrimitiveEntity::getBoundingBox() const
 {
	return m_boundingBox;
 }

void PrimitiveEntity::update(const world::UpdateParams& update)
{
}

    }
}