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
	int32_t m_grainIndex;
	Ref< ISoundBufferCursor > m_grainCursor;

	virtual void setParameter(handle_t id, float parameter)
	{
		if (m_grainCursor)
			m_grainCursor->setParameter(id, parameter);
	}

	virtual void reset()
	{
		if (m_grainCursor)
			m_grainCursor->reset();
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SequenceGrain", SequenceGrain, IGrain)

SequenceGrain::SequenceGrain(const RefArray< IGrain >& grains)
:	m_grains(grains)
{
}

Ref< ISoundBufferCursor > SequenceGrain::createCursor() const
{
	if (m_grains.empty())
		return 0;

	Ref< SequenceGrainCursor > cursor = new SequenceGrainCursor();
	cursor->m_grainIndex = 0;
	cursor->m_grainCursor = m_grains[0]->createCursor();

	return cursor->m_grainCursor ? cursor : 0;
}

void SequenceGrain::updateCursor(ISoundBufferCursor* cursor) const
{
	SequenceGrainCursor* sequenceCursor = static_cast< SequenceGrainCursor* >(cursor);
	const IGrain* grain = m_grains[sequenceCursor->m_grainIndex];
	return grain->updateCursor(sequenceCursor->m_grainCursor);
}

const IGrain* SequenceGrain::getCurrentGrain(const ISoundBufferCursor* cursor) const
{
	const SequenceGrainCursor* sequenceCursor = static_cast< const SequenceGrainCursor* >(cursor);
	const IGrain* grain = m_grains[sequenceCursor->m_grainIndex];
	return grain->getCurrentGrain(sequenceCursor->m_grainCursor);
}

bool SequenceGrain::getBlock(ISoundBufferCursor* cursor, const ISoundMixer* mixer, SoundBlock& outBlock) const
{
	SequenceGrainCursor* sequenceCursor = static_cast< SequenceGrainCursor* >(cursor);

	int32_t ngrains = int32_t(m_grains.size());
	if (sequenceCursor->m_grainIndex >= ngrains)
		return false;

	IGrain* grain = m_grains[sequenceCursor->m_grainIndex];

	for (;;)
	{
		if (grain->getBlock(
			sequenceCursor->m_grainCursor,
			mixer,
			outBlock
		))
			break;

		if (++sequenceCursor->m_grainIndex >= ngrains)
			return false;

		grain = m_grains[sequenceCursor->m_grainIndex];

		sequenceCursor->m_grainCursor = grain->createCursor();
	}

	return true;
}

	}
}
