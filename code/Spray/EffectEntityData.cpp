#include "Core/Serialization/ISerializer.h"
#include "Spray/Effect.h"
#include "Spray/EffectEntity.h"
#include "Spray/EffectEntityData.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spray.EffectEntityData", 0, EffectEntityData, world::EntityData)

Ref< EffectEntity > EffectEntityData::createEntity(resource::IResourceManager* resourceManager, sound::ISoundPlayer* soundPlayer) const
{
	resource::Proxy< Effect > effect;
	if (!resourceManager->bind(m_effect, effect))
		return 0;

	return new EffectEntity(
		getTransform(),
		effect,
		soundPlayer
	);
}

void EffectEntityData::serialize(ISerializer& s)
{
	world::EntityData::serialize(s);
	s >> resource::Member< Effect >(L"effect", m_effect);
}

	}
}
