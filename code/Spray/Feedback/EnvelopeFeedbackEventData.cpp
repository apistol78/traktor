#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Spray/Feedback/EnvelopeFeedbackEventData.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.EnvelopeFeedbackEventData", 0, EnvelopeFeedbackEventData, world::IEntityEventData)

EnvelopeFeedbackEventData::EnvelopeFeedbackEventData()
:	m_type(FbtNone)
{
}

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

EnvelopeFeedbackEventData::TimedValue::TimedValue()
:	at(0.0f)
,	value(0.0f)
{
}

void EnvelopeFeedbackEventData::TimedValue::serialize(ISerializer& s)
{
	s >> Member< float >(L"at", at);
	s >> Member< float >(L"value", value);
}

	}
}
