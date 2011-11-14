#include "Core/Math/Const.h"
#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Physics/AxisJointDesc.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.AxisJointDesc", 0, AxisJointDesc, JointDesc)

AxisJointDesc::AxisJointDesc()
:	m_axis(1.0f, 0.0f, 0.0f, 0.0f)
{
}

void AxisJointDesc::setAxis(const Vector4& axis)
{
	m_axis = axis;
}

const Vector4& AxisJointDesc::getAxis() const
{
	return m_axis;
}

bool AxisJointDesc::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"axis", m_axis, AttributeDirection());
	return true;
}

	}
}
