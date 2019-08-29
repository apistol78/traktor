#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

namespace traktor
{
    namespace model
    {

class Model;

    }

    namespace shape
    {

/*! Generate model from source object, such as mesh, terrain, spline, solid etc.
 * \ingroup Shape
 */
class IModelGenerator : public Object
{
    T_RTTI_CLASS;

public:
    virtual TypeInfoSet getSupportedTypes() const = 0;

    virtual Ref< model::Model > createModel(const Object* source) const = 0;
};

    }
}