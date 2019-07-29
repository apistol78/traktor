#include "Shape/Editor/Bake/TracerOutput.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.TracerOutput", TracerOutput, Object)

TracerOutput::TracerOutput(
	const std::wstring& name,
	int32_t priority,
    const model::Model* model,
    const Guid& lightmapId
)
:	m_name(name)
,	m_priority(priority)
,   m_model(model)
,   m_lightmapId(lightmapId)
{
}

    }
}