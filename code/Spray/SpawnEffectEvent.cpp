#include "Spray/EffectComponent.h"
#include "Spray/SpawnEffectEvent.h"
#include "Spray/SpawnEffectEventInstance.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.SpawnEffectEvent", SpawnEffectEvent, world::IEntityEvent)

SpawnEffectEvent::SpawnEffectEvent(
	sound::ISoundPlayer* soundPlayer,
	const resource::Proxy< Effect >& effect,
	const Transform& transform,
	bool follow,
	bool useRotation
)
:	m_soundPlayer(soundPlayer)
,	m_effect(effect)
,	m_transform(transform)
,	m_follow(follow)
,	m_useRotation(useRotation)
{
}

Ref< world::IEntityEventInstance > SpawnEffectEvent::createInstance(world::IEntityEventManager* eventManager, world::Entity* sender, const Transform& Toffset) const
{
	Ref< EffectComponent > effectComponent = new EffectComponent(
		m_effect,
		eventManager,
		m_soundPlayer
	);
	return new SpawnEffectEventInstance(this, sender, Toffset * m_transform, effectComponent);
}

	}
}
