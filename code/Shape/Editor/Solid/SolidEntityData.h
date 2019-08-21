#pragma once

#include "Core/RefArray.h"
#include "World/EntityData.h"

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

class PrimitiveEntityData;

/*! Solid geometry entity data.
 *
 * Contain solid geometry primitives along with operation (intersection, union, difference etc).
 */
class T_DLLCLASS SolidEntityData : public world::EntityData
{
    T_RTTI_CLASS;

public:
    virtual void serialize(ISerializer& s) override final;

    const RefArray< PrimitiveEntityData >& getPrimitives() const { return m_primitives; }

private:
    RefArray< PrimitiveEntityData > m_primitives;
};

    }
}