#include "Sound/Sound.h"
#include "Sound/Resound/BankBuffer.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.BankBuffer", BankBuffer, ISoundBuffer)

BankBuffer::BankBuffer(std::vector< resource::Proxy< Sound > >& sounds)
:	m_sounds(sounds)
{
}

void BankBuffer::reset()
{
	int32_t index = int32_t(std::rand() % m_sounds.size());
	if (m_sounds[index].validate())
		m_activeSound = m_sounds[index];
	else
		m_activeSound = 0;
}

bool BankBuffer::getBlock(double time, SoundBlock& outBlock)
{
	if (m_activeSound)
		return m_activeSound->getSoundBuffer()->getBlock(time, outBlock);
	else
		return false;
}

	}
}
