#pragma once

#include "Core/Ref.h"
#include "Core/Container/AlignedVector.h"
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

class PrimitiveEntity;

/*!
 */
class T_DLLCLASS PrimitiveEntityData : public world::EntityData
{
    T_RTTI_CLASS;

public:
    Ref< PrimitiveEntity > createEntity() const;

    virtual void serialize(ISerializer& s) override;

private:
    AlignedVector< Vector4 > m_vertices;
    AlignedVector< uint32_t > m_indices;
};

    }
}