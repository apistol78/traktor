/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Spray/Feedback/EnvelopeFeedbackEventData.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.EnvelopeFeedbackEventData", 0, EnvelopeFeedbackEventData, world::IEntityEventData)

void EnvelopeFeedbackEventData::serialize(ISerializer& s)
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
	s >> MemberStaticArray< std::vector< TimedValue >, 4, MemberStlVector< TimedValue, MemberComposite< TimedValue > > >(L"envelopes", m_envelopes);
}

void EnvelopeFeedbackEventData::TimedValue::serialize(ISerializer& s)
{
	s >> Member< float >(L"at", at);
	s >> Member< float >(L"value", value);
}

}
