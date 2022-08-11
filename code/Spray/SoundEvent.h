#pragma once

#include "Resource/Proxy.h"
#include "World/IEntityEvent.h"

namespace traktor
{
	namespace sound
	{

class ISoundPlayer;
class Sound;

	}

	namespace spray
	{

/*! Sound event.
 * \ingroup Spray
 */
class SoundEvent : public world::IEntityEvent
{
	T_RTTI_CLASS;

public:
	explicit SoundEvent(
		sound::ISoundPlayer* soundPlayer,
		const resource::Proxy< sound::Sound >& sound,
		bool positional,
		bool follow,
		bool autoStopFar
	);

	virtual Ref< world::IEntityEventInstance > createInstance(world::EntityEventManager* eventManager, world::Entity* sender, const Transform& Toffset) const override final;

private:
	sound::ISoundPlayer* m_soundPlayer;
	resource::Proxy< sound::Sound > m_sound;
	bool m_positional;
	bool m_follow;
	bool m_autoStopFar;
};

	}
}

