#include <cstring>
#include "Core/Containers/CircularVector.h"
#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
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

template < typename ValueType, int Capacity >
class CircularBuffer
{
public:
	CircularBuffer()
	:	m_size(0)
	,	m_front(0)
	,	m_back(0)
	{
	}

	template < typename OutputType, typename OutputTranslator >
	uint32_t read(OutputType* data, uint32_t count, const OutputTranslator& translate)
	{
		if (count == 0)
			return 0;

		uint32_t nread = std::min(count, m_size);
		if (nread <= Capacity - m_front)
		{
			for (uint32_t i = 0; i < nread; ++i)
				data[i] = translate(*(m_data + m_front + i));

			m_front += nread;
			if (m_front == Capacity)
				m_front = 0;
		}
		else
		{
			uint32_t size1 = Capacity - m_front;

			for (uint32_t i = 0; i < size1; ++i)
				data[i] = translate(*(m_data + m_front + i));

			uint32_t size2 = nread - size1;

			for (uint32_t i = 0; i < size2; ++i)
				data[i + size1] = translate(*(m_data + i));

			m_front = size2;
		}

		m_size -= nread;
		return nread;
	}

	uint32_t write(const ValueType* data, uint32_t count)
	{
		if (!count)
			return 0;

		uint32_t nwrite = std::min(count, Capacity - m_size);
		if (nwrite <= Capacity - m_back)
		{
			std::memcpy(m_data + m_back, data, nwrite * sizeof(ValueType));
			m_back += nwrite;
			if (m_back == Capacity)
				m_back = 0;
		}
		else
		{
			uint32_t size1 = Capacity - m_back;
			std::memcpy(m_data + m_back, data, size1 * sizeof(ValueType));
			uint32_t size2 = nwrite - size1;
			std::memcpy(m_data, data + size1, size2 * sizeof(ValueType));
			m_back = size2;
		}

		m_size += nwrite;
		return nwrite;
	}

	uint32_t size() const
	{
		return m_size;
	}

private:
	ValueType m_data[Capacity];
	uint32_t m_size;
	uint32_t m_front;
	uint32_t m_back;
};

class VoiceSoundBufferCursor : public RefCountImpl< sound::ISoundBufferCursor >
{
public:
	enum { OutputSize = 128 };

	AutoArrayPtr< float, AllocatorFree > m_output;

	VoiceSoundBufferCursor()
	{
		m_output.reset((float*)getAllocator()->alloc((OutputSize + 128) * sizeof(float), 16, T_FILE_LINE));
	}

	virtual void setParameter(sound::handle_t id, float parameter) {}

	virtual void disableRepeat() {}

	virtual void reset() {}
};

struct CastSample
{
	float operator () (int16_t v) const { return v / 32767.0f; }
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
			uint32_t n = std::min< uint32_t >(VoiceSoundBufferCursor::OutputSize, m_samples.size());
			m_samples.read< float, CastSample >(voiceCursor->m_output.ptr(), n, CastSample());

			outBlock.samples[0] = voiceCursor->m_output.ptr();
			outBlock.samples[1] = voiceCursor->m_output.ptr();
			outBlock.samplesCount = n;
			outBlock.sampleRate = m_sampleRate;
			outBlock.maxChannel = 2;

			return true;
		}
		else
			return false;
	}

	void enqueueVoiceSamples(const int16_t* samples, uint32_t samplesCount, uint32_t sampleRate)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		m_samples.write(samples, samplesCount);
		m_sampleRate = sampleRate;
	}

private:
	mutable Semaphore m_lock;
	mutable CircularBuffer< int16_t, 128 * 1024 > m_samples;
	uint32_t m_sampleRate;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.VoiceChat", VoiceChat, IVoiceChat)

void VoiceChat::destroy()
{
	for (std::list< Transmission >::iterator i = m_transmissions.end(); i != m_transmissions.end(); ++i)
	{
		i->soundHandle->stop();
		i->soundHandle = 0;
	}

	m_transmissions.clear();

	if (m_provider)
	{
		m_provider->setCallback(0);
		m_provider = 0;
	}

	m_userCache = 0;
	m_soundPlayer = 0;
}

void VoiceChat::setSoundPlayer(sound::ISoundPlayer* soundPlayer)
{
	m_soundPlayer = soundPlayer;
}

void VoiceChat::setAudience(const RefArray< IUser >& audience)
{
	std::vector< uint64_t > audienceHandles(audience.size());
	for (uint32_t i = 0; i < audience.size(); ++i)
	{
		const User* userImpl = dynamic_type_cast< const User* >(audience[i]);
		if (userImpl)
			audienceHandles[i] = userImpl->m_handle;
	}
	m_provider->setAudience(audienceHandles);
}

void VoiceChat::beginTransmission()
{
	m_provider->beginTransmission();
}

void VoiceChat::endTransmission()
{
	m_provider->endTransmission();
}

void VoiceChat::setMute(bool mute)
{
	m_muted = mute;
}

bool VoiceChat::isTransmitting(IUser* user)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	for (std::list< Transmission >::const_iterator i = m_transmissions.begin(); i != m_transmissions.end(); ++i)
	{
		if (i->user == user && i->soundHandle->isPlaying())
			return true;
	}
	return false;
}

VoiceChat::VoiceChat(IVoiceChatProvider* provider, UserCache* userCache)
:	m_provider(provider)
,	m_userCache(userCache)
,	m_category(0)
,	m_muted(false)
{
	m_category = sound::getParameterHandle(L"Voice");
	m_provider->setCallback(this);
}

VoiceChat::~VoiceChat()
{
	destroy();
}

void VoiceChat::onVoiceReceived(uint64_t fromUserHandle, const int16_t* samples, uint32_t samplesCount, uint32_t sampleRate)
{
	if (!m_soundPlayer || m_muted)
		return;

	User* fromUser = m_userCache->get(fromUserHandle);
	if (!fromUser)
		return;

#if defined(_DEBUG)
	std::wstring name;
	fromUser->getName(name);
	T_DEBUG(L"Received voice from \"" << name << L"\", " << samplesCount << L" sample(s), " << sampleRate << L" rate");
#endif

	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	for (std::list< Transmission >::iterator i = m_transmissions.begin(); i != m_transmissions.end(); ++i)
	{
		if (i->user == fromUser)
		{
			T_ASSERT (i->soundBuffer);
			i->soundBuffer->enqueueVoiceSamples(samples, samplesCount, sampleRate);

			if (!i->soundHandle->isPlaying())
			{
				Ref< sound::Sound > soundVoice = new sound::Sound(
					i->soundBuffer,
					m_category,
					1.0f,
					0.0f,
					1.0f,
					0.0f
				);
				i->soundHandle = m_soundPlayer->play(soundVoice, 0);
			}

			return;
		}
	}

	Transmission t;
	t.user = fromUser;
	t.soundBuffer = new VoiceSoundBuffer();
	t.soundBuffer->enqueueVoiceSamples(samples, samplesCount, sampleRate);

	Ref< sound::Sound > soundVoice = new sound::Sound(
		t.soundBuffer,
		m_category,
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
