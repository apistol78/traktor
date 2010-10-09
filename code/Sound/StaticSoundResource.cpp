#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberBitMask.h"
#include "Sound/Sound.h"
#include "Sound/StaticSoundBuffer.h"
#include "Sound/StaticSoundResource.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.StaticSoundResource", 0, StaticSoundResource, ISoundResource)

StaticSoundResource::StaticSoundResource()
:	m_sampleRate(0)
,	m_samplesCount(0)
,	m_channelsCount(0)
,	m_flags(0)
{
}

Ref< Sound > StaticSoundResource::createSound(resource::IResourceManager* resourceManager, db::Instance* resourceInstance) const
{
	Ref< StaticSoundBuffer > soundBuffer = new StaticSoundBuffer();
	if (!soundBuffer->create(this, resourceInstance))
	{
		log::error << L"Failed to create sound; unable to create static sound buffer" << Endl;
		return 0;
	}
	return new Sound(soundBuffer);
}

bool StaticSoundResource::serialize(ISerializer& s)
{
	const MemberBitMask::Bit c_Flags_bits[] =
	{
		{ L"SrfZLib", SrfZLib },
		{ 0 }
	};

	s >> Member< uint32_t >(L"sampleRate", m_sampleRate);
	s >> Member< uint32_t >(L"samplesCount", m_samplesCount);
	s >> Member< uint32_t >(L"channelsCount", m_channelsCount);
	s >> MemberBitMask(L"flags", m_flags, c_Flags_bits);
	
	return true;
}

	}
}
