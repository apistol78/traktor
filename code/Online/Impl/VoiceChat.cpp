#include "Core/Containers/CircularVector.h"
#include "Core/Thread/Acquire.h"
#include "Online/Impl/User.h"
#include "Online/Impl/UserCache.h"
#include "Online/Impl/VoiceChat.h"
#include "Online/Provider/IVoiceChatProvider.h"
#include "Sound/ISoundBuffer.h"
#include "Sound/Sound.h"
#include "Sound/Player/ISoundHandle.h"
#include "Sound/Player/ISoundPlayer.h"

namespace traktor
{
	namespace online
	{

class VoiceSoundBufferCursor : public RefCountImpl< sound::ISoundBufferCursor >
{
public:
	uint32_t m_position;
	float T_MATH_ALIGN16 m_output[1024];

	VoiceSoundBufferCursor()
	:	m_position(0)
	{
	}

	virtual void setParameter(sound::handle_t id, float parameter) {}

	virtual void disableRepeat() {}

	virtual void reset() {}
};

class VoiceSoundBuffer : public sound::ISoundBuffer
{
public:
	VoiceSoundBuffer()
	:	m_sampleRate(0)
	{
	}

	virtual Ref< sound::ISoundBufferCursor > createCursor() const
	{
		return new VoiceSoundBufferCursor();
	}

	virtual bool getBlock(sound::ISoundBufferCursor* cursor, const sound::ISoundMixer* mixer, sound::SoundBlock& outBlock) const
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

		VoiceSoundBufferCursor* voiceCursor = static_cast< VoiceSoundBufferCursor* >(cursor);
		if (m_samples.size() > 0)
		{
			uint32_t n = std::min< uint32_t >(sizeof_array(voiceCursor->m_output), m_samples.size());
			for (uint32_t i = 0; i < n; ++i)
			{
				voiceCursor->m_output[i] = m_samples.front() /32767.0f;
				m_samples.pop_front();
			}
			for (uint32_t i = n; i < sizeof_array(voiceCursor->m_output); ++i)
				voiceCursor->m_output[i] = 0.0f;

			outBlock.samples[0] = voiceCursor->m_output;
			outBlock.samplesCount = n;
			outBlock.sampleRate = m_sampleRate;
			outBlock.maxChannel = 1;

			return true;
		}
		else
			return false;
	}

	void enqueueVoiceSamples(const int16_t* samples, uint32_t samplesCount, uint32_t sampleRate)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		for (uint32_t i = 0; i < samplesCount; ++i)
			m_samples.push_back(samples[i]);
	}

private:
	mutable Semaphore m_lock;
	mutable CircularVector< int16_t, 131072 > m_samples;
	uint32_t m_sampleRate;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.VoiceChat", VoiceChat, IVoiceChat)

void VoiceChat::attachSoundPlayer(sound::ISoundPlayer* soundPlayer)
{
	m_soundPlayer = soundPlayer;
}

void VoiceChat::beginTransmission(const RefArray< IUser >& audience)
{
	std::vector< uint64_t > audienceHandles(audience.size());
	for (uint32_t i = 0; i < audience.size(); ++i)
	{
		const User* userImpl = dynamic_type_cast< const User* >(audience[i]);
		if (userImpl)
			audienceHandles[i] = userImpl->m_handle;
	}
	m_provider->beginTransmission(audienceHandles);
}

void VoiceChat::endTransmission()
{
	m_provider->endTransmission();
}

void VoiceChat::mute()
{
}

bool VoiceChat::isTransmitting(IUser* user)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	for (std::list< Transmission >::const_iterator i = m_transmissions.begin(); i != m_transmissions.end(); ++i)
	{
		if (i->user == user)
			return true;
	}
	return false;
}

VoiceChat::VoiceChat(IVoiceChatProvider* provider, UserCache* userCache)
:	m_provider(provider)
,	m_userCache(userCache)
{
	m_provider->setCallback(this);
}

VoiceChat::~VoiceChat()
{
	m_provider->setCallback(0);

	for (std::list< Transmission >::iterator i = m_transmissions.end(); i != m_transmissions.end(); ++i)
	{
		i->soundHandle->stop();
		i->soundHandle = 0;
	}

	m_transmissions.clear();
}

void VoiceChat::onVoiceReceived(uint64_t fromUserHandle, const int16_t* samples, uint32_t samplesCount, uint32_t sampleRate)
{
	if (!m_soundPlayer)
		return;

	User* fromUser = m_userCache->get(fromUserHandle);
	if (!fromUser)
		return;

	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	for (std::list< Transmission >::const_iterator i = m_transmissions.begin(); i != m_transmissions.end(); ++i)
	{
		if (i->user == fromUser)
		{
			T_ASSERT (i->soundBuffer);
			i->soundBuffer->enqueueVoiceSamples(samples, samplesCount, sampleRate);
			return;
		}
	}

	Transmission t;
	t.user = fromUser;
	t.soundBuffer = new VoiceSoundBuffer();
	t.soundBuffer->enqueueVoiceSamples(samples, samplesCount, sampleRate);

	Ref< sound::Sound > soundVoice = new sound::Sound(
		t.soundBuffer,
		0,
		1.0f,
		0.0f,
		1.0f,
		0.0f
	);
	t.soundHandle = m_soundPlayer->play(soundVoice, 0);

	if (t.soundHandle)
		m_transmissions.push_back(t);
}

	}
}
