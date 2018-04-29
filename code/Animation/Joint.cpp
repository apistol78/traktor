/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.Joint", 1, Joint, ISerializable)

Joint::Joint()
:	m_parent(-1)
,	m_transform(Transform::identity())
,	m_radius(0.1f)
{
}

void Joint::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"parent", m_parent);
	s >> Member< std::wstring >(L"name", m_name);
	s >> MemberComposite< Transform >(L"transform", m_transform);
	s >> Member< float >(L"radius", m_radius, AttributeRange(0.0f));

	if (s.getVersion< Joint >() < 1)
	{
	
		bool enableLimits;
		s >> Member< bool >(L"enableLimits", enableLimits);
		
		float twistLimit;
		s >> Member< float >(L"twistLimit", twistLimit, AttributeRange(-PI, PI));

		Vector2 coneLimit;
		s >> Member< Vector2 >(L"coneLimit", coneLimit);
	}
}

	}
}
