#include "Shape/Editor/Bake/TracerIrradiance.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.TracerIrradiance", TracerIrradiance, Object)

TracerIrradiance::TracerIrradiance(
    db::Instance* irradianceInstance,
    const Aabb3& boundingBox
)
:   m_irradianceInstance(irradianceInstance)
,	m_boundingBox(boundingBox)
{
}

    }
}