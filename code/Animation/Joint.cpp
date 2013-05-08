#include "Animation/Joint.h"
#include "Core/Math/Const.h"
#include "Core/Serialization/AttributePoint.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.Joint", 0, Joint, ISerializable)

Joint::Joint()
:	m_parent(-1)
,	m_transform(Transform::identity())
,	m_radius(0.0f)
,	m_enableLimits(false)
,	m_twistLimit(0.0f)
,	m_coneLimit(0.0f, 0.0f)
{
}

void Joint::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"parent", m_parent);
	s >> Member< std::wstring >(L"name", m_name);
	s >> MemberComposite< Transform >(L"transform", m_transform);
	s >> Member< float >(L"radius", m_radius, AttributeRange(0.0f));
	s >> Member< bool >(L"enableLimits", m_enableLimits);
	s >> Member< float >(L"twistLimit", m_twistLimit, AttributeRange(-PI, PI));
	s >> Member< Vector2 >(L"coneLimit", m_coneLimit);
}

	}
}
