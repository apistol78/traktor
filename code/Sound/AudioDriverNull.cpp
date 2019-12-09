#include "Sound/AudioDriverNull.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/Thread.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.AudioDriverNull", 0, AudioDriverNull, IAudioDriver)

bool AudioDriverNull::create(const SystemApplication& sysapp, const AudioDriverCreateDesc& desc, Ref< IAudioMixer >& outMixer)
{
	m_desc = desc;
	return true;
}

void AudioDriverNull::destroy()
{
}

void AudioDriverNull::wait()
{
	int32_t ms = int32_t(m_desc.frameSamples * 1000L / m_desc.sampleRate);
	ThreadManager::getInstance().getCurrentThread()->sleep(ms);
}

void AudioDriverNull::submit(const SoundBlock& soundBlock)
{
}

	}
}
