#include "Shape/Spline/ControlPointComponent.h"
#include "Shape/Spline/ControlPointComponentData.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.shape.ControlPointComponentData", 0, ControlPointComponentData, world::IEntityComponentData)

Ref< ControlPointComponent > ControlPointComponentData::createComponent() const
{
	return new ControlPointComponent();
}

void ControlPointComponentData::serialize(ISerializer& s)
{
}

	}
}
