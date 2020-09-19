#include "Shape/Editor/Spline/SplineLayerComponentData.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.SplineLayerComponentData", SplineLayerComponentData, world::IEntityComponentData)

void SplineLayerComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

	}
}
