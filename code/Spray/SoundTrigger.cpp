#include "Spray/SoundTrigger.h"
#include "Spray/SoundTriggerInstance.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.SoundTrigger", SoundTrigger, ITrigger)

SoundTrigger::SoundTrigger(const resource::Proxy< sound::Sound >& sound, bool follow, bool repeat)
:	m_sound(sound)
,	m_follow(follow)
,	m_repeat(repeat)
{
}

Ref< ITriggerInstance > SoundTrigger::createInstance() const
{
	return new SoundTriggerInstance(m_sound, m_follow, m_repeat);
}

	}
}
