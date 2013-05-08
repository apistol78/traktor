#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Spray/SpawnEffectEvent.h"
#include "Spray/SpawnEffectEventData.h"
#include "World/EntityData.h"
#include "World/IEntityBuilder.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spray.SpawnEffectEventData", 0, SpawnEffectEventData, world::IEntityEventData)

SpawnEffectEventData::SpawnEffectEventData()
:	m_follow(true)
{
}

Ref< world::IEntityEvent > SpawnEffectEventData::create(const world::IEntityBuilder* entityBuilder) const
{
	return new SpawnEffectEvent(entityBuilder->getCompositeEntityBuilder(), m_effectData, m_follow);
}

void SpawnEffectEventData::serialize(ISerializer& s)
{
	s >> MemberRef< world::EntityData >(L"effectData", m_effectData);
	s >> Member< bool >(L"follow", m_follow);
}

	}
}
