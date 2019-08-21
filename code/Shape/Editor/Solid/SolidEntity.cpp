#include "Shape/Editor/Solid/PrimitiveEntity.h"
#include "Shape/Editor/Solid/SolidEntity.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.SolidEntity", SolidEntity, world::GroupEntity)

SolidEntity::SolidEntity(const Transform& transform)
:   world::GroupEntity(transform)
{
}

void SolidEntity::update(const world::UpdateParams& update)
{
    world::GroupEntity::update(update);

    RefArray< PrimitiveEntity > primitiveEntities;
    getEntitiesOf< PrimitiveEntity >(primitiveEntities);

    // Check if any child entity is dirty and if so update our preview geometry.
    bool dirty = false;
    for (auto primitiveEntity : primitiveEntities)
    {
        dirty |= primitiveEntity->isDirty();
        primitiveEntity->resetDirty();
    }
    if (dirty)
    {
        // \tbd
    }
}

    }
}
