#include "Shape/Editor/Bake/TracerLight.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.TracerLight", TracerLight, Object)

TracerLight::TracerLight(const Light& light)
:   m_light(light)
{
}

    }
}