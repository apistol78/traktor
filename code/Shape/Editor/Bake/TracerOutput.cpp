#include "Shape/Editor/Bake/TracerOutput.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.TracerOutput", TracerOutput, Object)

TracerOutput::TracerOutput(
    const model::Model* model,
    const Guid& lightmapId
)
:   m_model(model)
,   m_lightmapId(lightmapId)
{
}

    }
}