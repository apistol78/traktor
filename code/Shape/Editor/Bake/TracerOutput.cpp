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
	const Transform& transform,
	const Guid& lightmapDiffuseId,
	const Guid& lightmapDirectionalId,
	int32_t lightmapSize
)
:	m_name(name)
,	m_priority(priority)
,   m_model(model)
,	m_transform(transform)
,   m_lightmapDiffuseId(lightmapDiffuseId)
,   m_lightmapDirectionalId(lightmapDirectionalId)
,	m_lightmapSize(lightmapSize)
{
}

    }
}