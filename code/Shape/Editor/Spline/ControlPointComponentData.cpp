#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Shape/Editor/Spline/ControlPointComponent.h"
#include "Shape/Editor/Spline/ControlPointComponentData.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.shape.ControlPointComponentData", 1, ControlPointComponentData, world::IEntityComponentData)

Ref< ControlPointComponent > ControlPointComponentData::createComponent() const
{
	return new ControlPointComponent(m_scale);
}

void ControlPointComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void ControlPointComponentData::serialize(ISerializer& s)
{
	if (s.getVersion< ControlPointComponentData >() >= 1)
		s >> Member< float >(L"scale", m_scale, AttributeUnit(UnitType::Percent));
}

	}
}
