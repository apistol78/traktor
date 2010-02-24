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

BankBuffer::BankBuffer(const RefArray< IGrain >& grains)
:	m_grains(grains)
{
}

Ref< ISoundBufferCursor > BankBuffer::createCursor() const
{
	Ref< BankBufferCursor > bankCursor = new BankBufferCursor();

	bankCursor->m_grainIndex = 0;
	bankCursor->m_grainCursor = m_grains[0]->createCursor();

	return bankCursor->m_grainCursor ? bankCursor : 0;
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

		bankCursor->m_grainCursor = grain->createCursor();
	}

	return true;
}

	}
}
