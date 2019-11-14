#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/AttributePoint.h"
#include "Core/Serialization/AttributeUnit.h"
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
,	m_anchor(0.0f, 1.0f, 0.0f, 1.0f)
,	m_axis(0.0f, 1.0f, 0.0f, 0.0f)
,	m_suspensionLength(0.1f, 1.0f)
,	m_suspensionSpring(5.0f)
,	m_suspensionDamping(5.0f)
,	m_rollingFriction(1.0f)
,	m_sideFriction(100.0f)
,	m_slipCornerForce(10.0f)
{
}

void WheelData::serialize(ISerializer& s)
{
	s >> Member< bool >(L"steer", m_steer);
	s >> Member< bool >(L"drive", m_drive);
	s >> Member< float >(L"radius", m_radius, AttributeUnit(AuMetres));
	s >> Member< Vector4 >(L"anchor", m_anchor, AttributePoint());
	s >> Member< Vector4 >(L"axis", m_axis, AttributeDirection());
	s >> MemberComposite< Range< float > >(L"suspensionLength", m_suspensionLength, AttributeUnit(AuMetres));
	s >> Member< float >(L"suspensionSpring", m_suspensionSpring);
	s >> Member< float >(L"suspensionDamping", m_suspensionDamping);
	s >> Member< float >(L"rollingFriction", m_rollingFriction);
	s >> Member< float >(L"sideFriction", m_sideFriction);
	s >> Member< float >(L"slipCornerForce", m_slipCornerForce);
}

	}
}
