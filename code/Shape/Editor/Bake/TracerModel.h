#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
    namespace model
    {

class Model;

    }

    namespace shape
    {

class T_DLLCLASS TracerModel : public Object
{
    T_RTTI_CLASS;

public:
    TracerModel(const model::Model* model);

    const model::Model* getModel() const { return m_model; }

private:
    Ref< const model::Model > m_model;
};

    }
}