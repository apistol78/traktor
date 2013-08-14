#include "Spray/EffectEntity.h"
#include "Spray/SpawnEffectEvent.h"
#include "Spray/SpawnEffectEventInstance.h"
#include "World/IEntityBuilder.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.SpawnEffectEvent", SpawnEffectEvent, world::IEntityEvent)

SpawnEffectEvent::SpawnEffectEvent(
	const world::IEntityBuilder* entityBuilder,
	const world::EntityData* effectData,
	bool follow,
	bool useRotation
)
:	m_entityBuilder(entityBuilder)
,	m_effectData(effectData)
,	m_follow(follow)
,	m_useRotation(useRotation)
{
}

Ref< world::IEntityEventInstance > SpawnEffectEvent::createInstance(world::Entity* sender, const Transform& Toffset) const
{
	Ref< EffectEntity > effect = m_entityBuilder->create< EffectEntity >(m_effectData);
	if (effect)
		return new SpawnEffectEventInstance(this, sender, Toffset, effect);
	else
		return 0;
}

	}
}
