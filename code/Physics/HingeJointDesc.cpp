#include "Physics/HingeJointDesc.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Math/Const.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.HingeJointDesc", HingeJointDesc, JointDesc)

HingeJointDesc::HingeJointDesc()
:	m_anchor(0.0f, 0.0f, 0.0f, 1.0f)
,	m_axis(1.0f, 0.0f, 0.0f, 0.0f)
,	m_minAngle(0.0f)
,	m_maxAngle(0.0f)
{
}

void HingeJointDesc::setAnchor(const Vector4& anchor)
{
	m_anchor = anchor;
}

const Vector4& HingeJointDesc::getAnchor() const
{
	return m_anchor;
}

void HingeJointDesc::setAxis(const Vector4& axis)
{
	m_axis = axis;
}

const Vector4& HingeJointDesc::getAxis() const
{
	return m_axis;
}

void HingeJointDesc::setAngles(float minAngle, float maxAngle)
{
	m_minAngle = minAngle;
	m_maxAngle = maxAngle;
}

void HingeJointDesc::getAngles(float& outMinAngle, float& outMaxAngle) const
{
	outMinAngle = m_minAngle;
	outMaxAngle = m_maxAngle;
}

bool HingeJointDesc::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"anchor", m_anchor);
	s >> Member< Vector4 >(L"axis", m_axis);
	s >> Member< float >(L"minAngle", m_minAngle, -PI, PI);
	s >> Member< float >(L"maxAngle", m_maxAngle, -PI, PI);
	return true;
}

	}
}
