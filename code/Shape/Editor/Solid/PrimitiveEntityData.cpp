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
    
    for (uint32_t i = 0; i < m_indices.size(); i += 4)
    {
        auto& w = entity->m_windings.push_back();
        w.resize(4);
        w[0] = m_vertices[m_indices[i + 3]];
        w[1] = m_vertices[m_indices[i + 2]];
        w[2] = m_vertices[m_indices[i + 1]];
        w[3] = m_vertices[m_indices[i + 0]];
    }
    
    entity->m_bspTree.build(
        entity->m_windings
    );

    return entity;
}

void PrimitiveEntityData::serialize(ISerializer& s)
{
    world::EntityData::serialize(s);

    s >> MemberAlignedVector< Vector4 >(L"vertices", m_vertices, AttributePrivate());
    s >> MemberAlignedVector< uint32_t >(L"indices", m_indices, AttributePrivate());
}

    }
}