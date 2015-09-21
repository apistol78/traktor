#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberSmallMap.h"
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
	SmallMap< std::wstring, resource::Proxy< sound::Sound > > sounds;
	for (SmallMap< std::wstring, resource::Id< sound::Sound > >::const_iterator i = m_sounds.begin(); i != m_sounds.end(); ++i)
	{
		if (!resourceManager->bind(i->second, sounds[i->first]))
			return 0;
	}
	return new SoundComponentInstance(soundPlayer, sounds);
}

void SoundComponent::serialize(ISerializer& s)
{
	s >> MemberSmallMap< std::wstring, resource::Id< sound::Sound >, Member< std::wstring >, resource::Member< sound::Sound > >(L"sounds", m_sounds);
}

	}
}
