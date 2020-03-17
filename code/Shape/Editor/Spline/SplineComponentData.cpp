#include "Shape/Editor/Spline/SplineComponent.h"
#include "Shape/Editor/Spline/SplineComponentData.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.shape.SplineComponentData", 0, SplineComponentData, world::IEntityComponentData)

Ref< SplineComponent > SplineComponentData::createComponent() const
{
	return new SplineComponent();
}

void SplineComponentData::serialize(ISerializer& s)
{
}

	}
}
