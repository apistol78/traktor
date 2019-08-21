#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Winding3.h"
#include "World/Entity/GroupEntity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
    namespace shape
    {

class T_DLLCLASS SolidEntity : public world::GroupEntity
{
    T_RTTI_CLASS;

public:
    SolidEntity(const Transform& transform = Transform::identity());

    virtual void update(const world::UpdateParams& update) override final;

    const AlignedVector< Winding3 >& getWindings() const { return m_windings; }

private:
    AlignedVector< Winding3 > m_windings;
};

    }
}