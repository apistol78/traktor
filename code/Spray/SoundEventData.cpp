#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Resource/Member.h"
#include "Sound/Sound.h"
#include "Spray/SoundEventData.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spray.SoundEventData", 1, SoundEventData, world::IEntityEventData)

void SoundEventData::serialize(ISerializer& s)
{
	T_FATAL_ASSERT(s.getVersion< SoundEventData >() >= 1);

	s >> resource::Member< sound::Sound >(L"sound", m_sound);
	s >> Member< bool >(L"positional", m_positional);
	s >> Member< bool >(L"follow", m_follow);
	s >> Member< bool >(L"autoStopFar", m_autoStopFar);
}

	}
}
