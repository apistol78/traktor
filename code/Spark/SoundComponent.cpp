#include "Core/Serialization/ISerializer.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Sound/Sound.h"
#include "Spark/SoundComponent.h"
#include "Spark/SoundComponentInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spark.SoundComponent", 0, SoundComponent, IComponent)

Ref< IComponentInstance > SoundComponent::createInstance(SpriteInstance* owner, resource::IResourceManager* resourceManager, sound::ISoundPlayer* soundPlayer) const
{
	resource::Proxy< sound::Sound > sound;
	if (!resourceManager->bind(m_sound, sound))
		return 0;

	return new SoundComponentInstance(soundPlayer, sound);
}

void SoundComponent::serialize(ISerializer& s)
{
	s >> resource::Member< sound::Sound >(L"sound", m_sound);
}

	}
}
