#pragma once

#include <functional>
#include "Core/Object.h"
#include "Core/Ref.h"

namespace traktor
{

class ISerializable;

    namespace world
    {

class EntityData;

    }

    namespace shape
    {

class Traverser : public Object
{
    T_RTTI_CLASS;

public:
    Traverser(ISerializable* object);

    void visit(const std::function< bool(Ref< world::EntityData >&) >& visitor);

private:
    Ref< ISerializable > m_object;
};

    }
}
