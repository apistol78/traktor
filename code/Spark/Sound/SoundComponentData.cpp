/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Sound/Sound.h"
#include "Spark/Context.h"
#include "Spark/Sound/SoundComponentData.h"
#include "Spark/Sound/SoundComponent.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spark.SoundComponentData", 0, SoundComponentData, IComponentData)

Ref< IComponent > SoundComponentData::createInstance(const Context* context, Sprite* owner) const
{
	SmallMap< std::wstring, resource::Proxy< sound::Sound > > sounds;
	for (SmallMap< std::wstring, resource::Id< sound::Sound > >::const_iterator i = m_sounds.begin(); i != m_sounds.end(); ++i)
	{
		if (!context->getResourceManager()->bind(i->second, sounds[i->first]))
			return 0;
	}
	return new SoundComponent(context->getSoundPlayer(), sounds);
}

void SoundComponentData::serialize(ISerializer& s)
{
	s >> MemberSmallMap< std::wstring, resource::Id< sound::Sound >, Member< std::wstring >, resource::Member< sound::Sound > >(L"sounds", m_sounds);
}

	}
}
