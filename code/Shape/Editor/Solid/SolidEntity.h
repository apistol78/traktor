#pragma once

#include "World/Entity/GroupEntity.h"

namespace traktor
{
    namespace shape
    {

class T_DLLCLAS SolidEntity : public world::GroupEntity
{
    T_RTTI_CLASS;

public:
    SolidEntity(const Transform& transform = Transform::identity());

    virtual void update(const world::UpdateParams& update) override final;
};

    }
}