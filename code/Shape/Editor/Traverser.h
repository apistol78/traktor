#pragma once

#include <functional>
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

class Traverser
{
public:
    static void visit(const ISerializable* object, const std::function< bool(const world::EntityData*) >& visitor);

    static void visit(ISerializable* object, const std::function< bool(Ref< world::EntityData >&) >& visitor);
};

    }
}
