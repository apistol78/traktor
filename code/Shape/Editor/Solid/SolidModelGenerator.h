#pragma once

#include "Shape/Editor/IModelGenerator.h"

namespace traktor
{
    namespace shape
    {

class SolidModelGenerator : public IModelGenerator
{
    T_RTTI_CLASS;

public:
    virtual TypeInfoSet getSupportedTypes() const override final;

    virtual Ref< model::Model > createModel(const Object* source) const override final;
};

    }
}