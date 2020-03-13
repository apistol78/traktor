#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Shape/Spline/ControlPointComponent.h"
#include "Shape/Spline/ControlPointComponentData.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.shape.ControlPointComponentData", 1, ControlPointComponentData, world::IEntityComponentData)

ControlPointComponentData::ControlPointComponentData()
:	m_scale(1.0f)
{
}

Ref< ControlPointComponent > ControlPointComponentData::createComponent() const
{
	return new ControlPointComponent(m_scale);
}

void ControlPointComponentData::serialize(ISerializer& s)
{
	s >> Member< float >(L"scale", m_scale, AttributeUnit(AuPercent));
}

	}
}
