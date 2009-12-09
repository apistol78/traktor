#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Sound/ISoundBuffer.h"
#include "Sound/Resound/SequenceGrain.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

struct SequenceGrainCursor : public RefCountImpl< ISoundBufferCursor >
{
	const BankBuffer* m_bank;
	int32_t m_grainIndex;
	Ref< ISoundBufferCursor > m_grainCursor;

	virtual void setCursor(double time)
	{
		m_grainCursor->setCursor(time);
	}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SequenceGrain", 0, SequenceGrain, IGrain)

Ref< ISoundBufferCursor > SequenceGrain::createCursor(const BankBuffer* bank) const
{
	if (m_grains.empty())
		return 0;

	Ref< SequenceGrainCursor > cursor = new SequenceGrainCursor();
	cursor->m_bank = bank;
	cursor->m_grainIndex = 0;
	cursor->m_grainCursor = m_grains[0]->createCursor(bank);

	return cursor;
}

bool SequenceGrain::getBlock(ISoundBufferCursor* cursor, SoundBlock& outBlock) const
{
	SequenceGrainCursor* sequenceCursor = static_cast< SequenceGrainCursor* >(cursor);
	IGrain* grain = m_grains[sequenceCursor->m_grainIndex];

	for (;;)
	{
		if (grain->getBlock(
			sequenceCursor->m_grainCursor,
			outBlock
		))
			break;

		if (++sequenceCursor->m_grainIndex >= int32_t(m_grains.size()))
			return false;

		grain = m_grains[sequenceCursor->m_grainIndex];

		sequenceCursor->m_grainCursor = grain->createCursor(sequenceCursor->m_bank);
	}

	return true;
}

bool SequenceGrain::serialize(ISerializer& s)
{
	return s >> MemberRefArray< IGrain >(L"grains", m_grains);
}

	}
}
