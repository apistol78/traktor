#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberRef.h"
#include "Model/Model.h"
#include "Shape/Editor/Solid/IShape.h"
#include "Shape/Editor/Solid/PrimitiveEntity.h"
#include "Shape/Editor/Solid/PrimitiveEntityData.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.shape.PrimitiveEntityData", 0, PrimitiveEntityData, world::EntityData)

PrimitiveEntityData::PrimitiveEntityData()
:   m_operation(BooleanOperation::BoUnion)
{
}

Ref< PrimitiveEntity > PrimitiveEntityData::createEntity(db::Database* database) const
{
    Ref< PrimitiveEntity > entity = new PrimitiveEntity(getTransform(), m_operation);
	if (m_shape)
	{
		Ref< const model::Model > m = m_shape->createModel(database);
		if (!m)
			return nullptr;

		entity->m_model = m;
		entity->m_boundingBox = m->getBoundingBox();
	}
    return entity;
}

void PrimitiveEntityData::serialize(ISerializer& s)
{
    const MemberEnum< BooleanOperation >::Key c_BooleanOperation_Keys[] =
    {
        { L"BoUnion", BooleanOperation::BoUnion },
        { L"BoIntersection", BooleanOperation::BoIntersection },
        { L"BoDifference", BooleanOperation::BoDifference },
        { 0 }
    };

    world::EntityData::serialize(s);

    s >> MemberEnum< BooleanOperation >(L"operation", m_operation, c_BooleanOperation_Keys);
	s >> MemberRef< IShape >(L"shape", m_shape);
}

    }
}