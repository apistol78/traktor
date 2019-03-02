#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Resource/Member.h"
#include "Spray/Effect.h"
#include "Spray/SpawnEffectEvent.h"
#include "Spray/SpawnEffectEventData.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spray.SpawnEffectEventData", 3, SpawnEffectEventData, world::IEntityEventData)

SpawnEffectEventData::SpawnEffectEventData()
:	m_transform(Transform::identity())
,	m_follow(true)
,	m_useRotation(true)
{
}

void SpawnEffectEventData::serialize(ISerializer& s)
{
	T_FATAL_ASSERT (s.getVersion() >= 2);
	s >> resource::Member< Effect >(L"effect", m_effect);

	if (s.getVersion() >= 3)
		s >> MemberComposite< Transform >(L"transform", m_transform);

	s >> Member< bool >(L"follow", m_follow);
	s >> Member< bool >(L"useRotation", m_useRotation);
}

	}
}
