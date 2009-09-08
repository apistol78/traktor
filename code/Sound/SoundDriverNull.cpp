#include "Sound/SoundDriverNull.h"
#include "Core/Serialization/Serializable.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/Thread.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.sound.SoundDriverNull", SoundDriverNull, ISoundDriver)

bool SoundDriverNull::create(const SoundDriverCreateDesc& desc)
{
	m_desc = desc;
	return true;
}

void SoundDriverNull::destroy()
{
}

void SoundDriverNull::wait()
{
	long ms = m_desc.frameSamples * 1000L / m_desc.sampleRate;
	ThreadManager::getInstance().getCurrentThread()->sleep(ms);
}

void SoundDriverNull::submit(const SoundBlock& soundBlock)
{
}

	}
}