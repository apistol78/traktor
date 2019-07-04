#pragma once

#include "Core/Object.h"
#include "Shape/Editor/Bake/Types.h"

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

class T_DLLCLASS TracerLight : public Object
{
    T_RTTI_CLASS;

public:
    TracerLight(const Light& light);

    const Light& getLight() const { return m_light; }

private:
    Light m_light;
};

    }
}