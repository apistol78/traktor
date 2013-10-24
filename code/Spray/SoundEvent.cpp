#include "Spray/SoundEvent.h"
#include "Spray/SoundEventInstance.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.SoundEvent", SoundEvent, world::IEntityEvent)

SoundEvent::SoundEvent(
	sound::ISoundPlayer* soundPlayer,
	const resource::Proxy< sound::Sound >& sound,
	bool positional,
	bool follow
)
:	m_soundPlayer(soundPlayer)
,	m_sound(sound)
,	m_positional(positional)
,	m_follow(follow)
{
}

Ref< world::IEntityEventInstance > SoundEvent::createInstance(world::IEntityEventManager* eventManager, world::Entity* sender, const Transform& Toffset) const
{
	if (m_soundPlayer)
		return new SoundEventInstance(sender, Toffset, m_soundPlayer, m_sound, m_positional, m_follow);
	else
		return 0;
}

	}
}
