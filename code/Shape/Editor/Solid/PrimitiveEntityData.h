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
    namespace shape
    {

class IShape;
class PrimitiveEntity;

/*!
 */
class T_DLLCLASS PrimitiveEntityData : public world::EntityData
{
    T_RTTI_CLASS;

public:
    PrimitiveEntityData();
    
    Ref< PrimitiveEntity > createEntity() const;

    virtual void serialize(ISerializer& s) override;

private:
    BooleanOperation m_operation;
	Ref< IShape > m_shape;
};

    }
}