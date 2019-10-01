#pragma once

#include "Core/Ref.h"
#include "Shape/Editor/Solid/SolidTypes.h"
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
    namespace db
    {

class Database;

    }

    namespace shape
    {

class IShape;
class PrimitiveEntity;

/*! Primitive entity data.
 * \ingroup Shape
 */
class T_DLLCLASS PrimitiveEntityData : public world::EntityData
{
    T_RTTI_CLASS;

public:
    PrimitiveEntityData();
    
    Ref< PrimitiveEntity > createEntity(db::Database* database) const;

    virtual void serialize(ISerializer& s) override;

    BooleanOperation getOperation() const { return m_operation; }

    const IShape* getShape() const { return m_shape; }

private:
    friend class PrimitiveEditModifier;

    BooleanOperation m_operation;
	Ref< IShape > m_shape;
};

    }
}