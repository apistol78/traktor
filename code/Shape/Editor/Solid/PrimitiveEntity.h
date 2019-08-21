#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/BspTree.h"
#include "Core/Math/Winding3.h"
#include "World/Entity.h"

namespace traktor
{
    namespace shape
    {

class T_DLLCLAS PrimitiveEntity : public world::Entity
{
    T_RTTI_CLASS;

public:
    PrimitiveEntity(const Transform& transform = Transform::identity());

    virtual void setTransform(const Transform& transform) override final;

    virtual bool getTransform(Transform& outTransform) const override final;

    virtual void update(const world::UpdateParams& update) override final;

    bool isDirty() const { return m_dirty; }

    void resetDirty() { m_dirty = false; }

    const AlignedVector< Winding3 >& getWindings() const { return m_windings; }

    const BspTree& getBspTree() const { return m_bspTree; }

protected:
    friend class PrimitiveEntityData;

    Transform m_transform;
    AlignedVector< Winding3 > m_windings;
    BspTree m_bspTree;
    bool m_dirty;
};

    }
}