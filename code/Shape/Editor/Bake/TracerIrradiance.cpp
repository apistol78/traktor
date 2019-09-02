#include "Shape/Editor/Bake/TracerIrradiance.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.TracerIrradiance", TracerIrradiance, Object)

TracerIrradiance::TracerIrradiance(
    const std::wstring& name,
    const Guid& irradianceGridId,
    const Aabb3& boundingBox
)
:	m_name(name)
,   m_irradianceGridId(irradianceGridId)
,	m_boundingBox(boundingBox)
{
}

    }
}