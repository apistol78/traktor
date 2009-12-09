#include "Sound/Sound.h"
#include "Sound/Resound/BankBuffer.h"
#include "Sound/Resound/IGrain.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

struct BankBufferCursor : public RefCountImpl< ISoundBufferCursor >
{
	int32_t m_grainIndex;
	Ref< ISoundBufferCursor > m_grainCursor;

	virtual void setCursor(double time)
	{
		m_grainCursor->setCursor(time);
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.BankBuffer", BankBuffer, ISoundBuffer)

BankBuffer::BankBuffer(
	const RefArray< IGrain >& grains,
	const RefArray< BankSound >& sounds
)
:	m_grains(grains)
,	m_sounds(sounds)
{
}

BankSound* BankBuffer::getSound(int32_t index) const
{
	if (index >= 0 && index < int32_t(m_sounds.size()))
		return m_sounds[index];
	else
		return 0;
}

Ref< ISoundBufferCursor > BankBuffer::createCursor() const
{
	Ref< BankBufferCursor > bankCursor = new BankBufferCursor();

	bankCursor->m_grainIndex = 0;
	bankCursor->m_grainCursor = m_grains[0]->createCursor(this);

	return bankCursor;
}

bool BankBuffer::getBlock(ISoundBufferCursor* cursor, SoundBlock& outBlock) const
{
	BankBufferCursor* bankCursor = static_cast< BankBufferCursor* >(cursor);
	IGrain* grain = m_grains[bankCursor->m_grainIndex];

	for (;;)
	{
		if (grain->getBlock(
			bankCursor->m_grainCursor,
			outBlock
		))
			break;

		if (++bankCursor->m_grainIndex >= int32_t(m_grains.size()))
			return false;

		grain = m_grains[bankCursor->m_grainIndex];

		bankCursor->m_grainCursor = grain->createCursor(this);
	}

	return true;
}

	}
}
