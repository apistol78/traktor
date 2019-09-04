#include "Shape/Editor/Bake/TracerModel.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.TracerModel", TracerModel, Object)

TracerModel::TracerModel(const model::Model* model, const Transform& transform)
:   m_model(model)
,	m_transform(transform)
{
}

    }
}