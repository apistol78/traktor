#include "Spray/EffectEntityData.h"
#include "Spray/EffectEntity.h"
#include "Spray/Effect.h"
#include "Core/Serialization/Serializer.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.spray.EffectEntityData", EffectEntityData, world::SpatialEntityData)

EffectEntity* EffectEntityData::createEntity() const
{
	return gc_new< EffectEntity >(
		cref(getTransform()),
		m_effect
	);
}

bool EffectEntityData::serialize(Serializer& s)
{
	if (!world::SpatialEntityData::serialize(s))
		return false;

	return s >> resource::Member< Effect >(L"effect", m_effect);
}

	}
}
