#include "Sound/SoundDriverNull.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/Thread.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SoundDriverNull", 0, SoundDriverNull, ISoundDriver)

bool SoundDriverNull::create(const SystemApplication& sysapp, const SoundDriverCreateDesc& desc, Ref< ISoundMixer >& outMixer)
{
	m_desc = desc;
	return true;
}

void SoundDriverNull::destroy()
{
}

void SoundDriverNull::wait()
{
	int32_t ms = int32_t(m_desc.frameSamples * 1000L / m_desc.sampleRate);
	ThreadManager::getInstance().getCurrentThread()->sleep(ms);
}

void SoundDriverNull::submit(const SoundBlock& soundBlock)
{
}

	}
}