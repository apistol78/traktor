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
    enum VisitorResult
    {
        VrContinue,
        VrSkip,
        VrFailed
    };

    static void visit(const ISerializable* object, const std::function< VisitorResult(const world::EntityData*) >& visitor);

    static void visit(ISerializable* object, const std::function< VisitorResult(Ref< world::EntityData >&) >& visitor);
};

    }
}
