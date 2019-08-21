#include "Shape/Editor/Solid/PrimitiveEntity.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.PrimitiveEntity", PrimitiveEntity, world::Entity)

PrimitiveEntity::PrimitiveEntity(const Transform& transform)
:   m_transform(transform)
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

void PrimitiveEntity::update(const world::UpdateParams& update)
{
}

    }
}