#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Shape/Editor/Solid/PrimitiveEntity.h"
#include "Shape/Editor/Solid/PrimitiveEntityData.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.shape.PrimitiveEntityData", 0, PrimitiveEntityData, world::EntityData)

Ref< PrimitiveEntity > PrimitiveEntityData::createEntity() const
{
    Ref< PrimitiveEntity > entity = new PrimitiveEntity();
    
    entity->m_transform = getTransform();
    entity->m_bspTree.build();

    return entity;
}

void PrimitiveEntityData::serialize(ISerializer& s)
{
    s >> MemberAlignedVector< Vector4 >(L"vertices", m_vertices, AttributePrivate());
    s >> MemberAlignedVector< uint32_t >(L"indices", m_indices, AttributePrivate());
}

    }
}