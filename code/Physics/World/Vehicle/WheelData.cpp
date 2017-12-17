#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/AttributePoint.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Physics/World/Vehicle/WheelData.h"

namespace traktor
{
	namespace physics
	{
	
T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.physics.WheelData", 0, WheelData, ISerializable)
	
WheelData::WheelData()
:	m_steer(false)
,	m_drive(false)
,	m_radius(1.0f)
,	m_anchor(0.0f, 0.0f, 0.0f, 1.0f)
,	m_axis(0.0f, 1.0f, 0.0f, 0.0f)
,	m_suspensionLength(0.1f, 1.0f)
,	m_suspensionSpringCoeff(1000.0f)
,	m_suspensionDampingCoeff(1000.0f)
,	m_rollingFrictionCoeff(5.0f)
,	m_sideFrictionCoeff(12.0f)
,	m_slipCornerForceCoeff(12.0f)
{
}

void WheelData::serialize(ISerializer& s)
{
	s >> Member< bool >(L"steer", m_steer);
	s >> Member< bool >(L"drive", m_drive);
	s >> Member< float >(L"radius", m_radius);
	s >> Member< Vector4 >(L"anchor", m_anchor, AttributePoint());
	s >> Member< Vector4 >(L"axis", m_axis, AttributeDirection());
	s >> MemberComposite< Range< float > >(L"suspensionLength", m_suspensionLength);
	s >> Member< float >(L"suspensionSpringCoeff", m_suspensionSpringCoeff);
	s >> Member< float >(L"suspensionDampingCoeff", m_suspensionDampingCoeff);
	s >> Member< float >(L"rollingFrictionCoeff", m_rollingFrictionCoeff);
	s >> Member< float >(L"sideFrictionCoeff", m_sideFrictionCoeff);
	s >> Member< float >(L"slipCornerForceCoeff", m_slipCornerForceCoeff);
}

	}
}
