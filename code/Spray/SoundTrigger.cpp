#include "Spray/SoundTrigger.h"
#include "Spray/SoundTriggerData.h"
#include "Spray/SoundTriggerInstance.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.SoundTrigger", SoundTrigger, ITrigger)

SoundTrigger::SoundTrigger(const SoundTriggerData* data, const resource::Proxy< sound::Sound >& sound)
:	m_data(data)
,	m_sound(sound)
{
}

Ref< ITriggerInstance > SoundTrigger::createInstance() const
{
	return new SoundTriggerInstance(
		m_sound,
		m_data->m_positional,
		m_data->m_follow,
		m_data->m_repeat
	);
}

	}
}
