#include "Spray/EffectEntityData.h"
#include "Spray/EffectEntity.h"
#include "Spray/Effect.h"
#include "Core/Serialization/Serializer.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_EDITABLE_CLASS(L"traktor.spray.EffectEntityData", EffectEntityData, world::SpatialEntityData)

EffectEntity* EffectEntityData::createEntity(resource::IResourceManager* resourceManager) const
{
	if (!resourceManager->bind(m_effect))
		return 0;

	return gc_new< EffectEntity >(
		resourceManager,
		cref(getTransform()),
		cref(m_effect)
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
