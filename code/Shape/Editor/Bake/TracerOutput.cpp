#include "Shape/Editor/Bake/TracerOutput.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.TracerOutput", TracerOutput, Object)

TracerOutput::TracerOutput(
	db::Instance* lightmapDiffuseInstance,
	db::Instance* lightmapDirectionalInstance,
    const model::Model* model,
	const Transform& transform,
	int32_t lightmapSize
)
:   m_lightmapDiffuseInstance(lightmapDiffuseInstance)
,   m_lightmapDirectionalInstance(lightmapDirectionalInstance)
,   m_model(model)
,	m_transform(transform)
,	m_lightmapSize(lightmapSize)
{
}

    }
}