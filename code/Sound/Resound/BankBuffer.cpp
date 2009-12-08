#include "Sound/Sound.h"
#include "Sound/Resound/BankBuffer.h"
#include "Sound/Resound/BankBufferCursor.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.BankBuffer", BankBuffer, ISoundBuffer)

BankBuffer::BankBuffer(std::vector< resource::Proxy< Sound > >& sounds)
:	m_sounds(sounds)
{
}

Ref< ISoundBufferCursor > BankBuffer::createCursor()
{
	int32_t index = int32_t(std::rand() % m_sounds.size());

	if (!m_sounds[index].validate())
		return 0;

	Ref< ISoundBuffer > activeBuffer = m_sounds[index]->getSoundBuffer();
	T_ASSERT (activeBuffer);

	Ref< ISoundBufferCursor > activeCursor = activeBuffer->createCursor();
	if (!activeCursor)
		return 0;

	return new BankBufferCursor(activeBuffer, activeCursor);
}

bool BankBuffer::getBlock(const ISoundBufferCursor* cursor, SoundBlock& outBlock)
{
	const BankBufferCursor* bankCursor = static_cast< const BankBufferCursor* >(cursor);

	ISoundBuffer* activeBuffer = bankCursor->getActiveBuffer();
	T_ASSERT (activeBuffer);

	return activeBuffer->getBlock(
		bankCursor->getActiveCursor(),
		outBlock
	);
}

	}
}
