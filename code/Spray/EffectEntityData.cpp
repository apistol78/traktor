#include "Spray/EffectEntityData.h"
#include "Spray/EffectEntity.h"
#include "Spray/Effect.h"
#include "Core/Serialization/ISerializer.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spray.EffectEntityData", 0, EffectEntityData, world::SpatialEntityData)

Ref< EffectEntity > EffectEntityData::createEntity(resource::IResourceManager* resourceManager, sound::SoundSystem* soundSystem, sound::SurroundEnvironment* surroundEnvironment) const
{
	if (!resourceManager->bind(m_effect))
		return 0;

	return new EffectEntity(
		getTransform(),
		m_effect,
		soundSystem,
		surroundEnvironment
	);
}

bool EffectEntityData::serialize(ISerializer& s)
{
	if (!world::SpatialEntityData::serialize(s))
		return false;

	return s >> resource::Member< Effect >(L"effect", m_effect);
}

	}
}
