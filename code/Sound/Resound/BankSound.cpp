#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Sound/ISoundResource.h"
#include "Sound/Sound.h"
#include "Sound/Resound/BankSound.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.BankSound", 0, BankSound, ISerializable)

BankSound::BankSound()
:	m_pitch(1.0f)
{
}

BankSound::BankSound(const resource::Proxy< Sound >& sound, float pitch)
:	m_sound(sound)
,	m_pitch(pitch)
{
}

bool BankSound::bind(resource::IResourceManager* resourceManager) const
{
	return resourceManager->bind(m_sound);
}

resource::Proxy< Sound >& BankSound::getSound()
{
	return m_sound;
}

void BankSound::setPitch(float pitch)
{
	m_pitch = pitch;
}

float BankSound::getPitch() const
{
	return m_pitch;
}

bool BankSound::serialize(ISerializer& s)
{
	s >> resource::Member< Sound, ISoundResource >(L"sound", m_sound);
	s >> Member< float >(L"pitch", m_pitch, 0.5f, 2.0f);
	return true;
}

	}
}
