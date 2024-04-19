/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Spray/Feedback/OscillateFeedbackEventData.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.OscillateFeedbackEventData", 1, OscillateFeedbackEventData, world::IEntityEventData)

void OscillateFeedbackEventData::serialize(ISerializer& s)
{
	const MemberEnum< FeedbackType >::Key c_FeedbackType_keys[] =
	{
		{ L"FbtNone", FbtNone },
		{ L"FbtCamera", FbtCamera },
		{ L"FbtImageProcess", FbtImageProcess },
		{ L"FbtUI", FbtUI },
		{ 0 }
	};

	s >> MemberEnum< FeedbackType >(L"type", m_type, c_FeedbackType_keys);
	s >> MemberStaticArray< OscillatingValue, 4, MemberComposite< OscillatingValue > >(L"values", m_values);
}

void OscillateFeedbackEventData::OscillatingValue::serialize(ISerializer& s)
{
	s >> Member< float >(L"duration", duration, AttributeRange(0.0f));
	s >> Member< int32_t >(L"frequency", frequency, AttributeRange(0));
	s >> Member< float >(L"magnitude", magnitude, AttributeRange(0.0f));
	if (s.getVersion() >= 1)
		s >> Member< float >(L"noise", noise, AttributeRange(0.0f, 1.0f));
}

}
