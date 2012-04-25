#if !defined(WINCE)
#	include <ctime>
#else
#	include <time_ce.h>
#endif
#include "Sound/ISoundBuffer.h"
#include "Sound/Resound/RandomGrain.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

struct RandomGrainCursor : public RefCountImpl< ISoundBufferCursor >
{
	Ref< IGrain > m_grain;
	Ref< ISoundBufferCursor > m_grainCursor;

	virtual void reset()
	{
		if (m_grainCursor)
			m_grainCursor->reset();
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.RandomGrain", RandomGrain, IGrain)

RandomGrain::RandomGrain(
	const RefArray< IGrain >& grains,
	bool humanize
)
:	m_grains(grains)
#if !defined(WINCE)
,	m_random(uint32_t(clock()))
#else
,	m_random(uint32_t(clock_ce()))
#endif
,	m_humanize(humanize)
,	m_last(-1)
{
}

Ref< ISoundBufferCursor > RandomGrain::createCursor() const
{
	int32_t index = int32_t(m_random.nextFloat() * m_grains.size());
	if (m_humanize && m_grains.size() >= 2)
	{
		while (index == m_last)
			index = int32_t(m_random.nextFloat() * m_grains.size());
		m_last = index;
	}

	Ref< RandomGrainCursor > cursor = new RandomGrainCursor();
	cursor->m_grain = m_grains[index];
	cursor->m_grainCursor = m_grains[index]->createCursor();

	return cursor;
}

void RandomGrain::updateCursor(ISoundBufferCursor* cursor) const
{
	RandomGrainCursor* randomCursor = static_cast< RandomGrainCursor* >(cursor);
	return randomCursor->m_grain->updateCursor(randomCursor->m_grainCursor);
}

const IGrain* RandomGrain::getCurrentGrain(ISoundBufferCursor* cursor) const
{
	RandomGrainCursor* randomCursor = static_cast< RandomGrainCursor* >(cursor);
	return randomCursor->m_grain->getCurrentGrain(randomCursor->m_grainCursor);
}

bool RandomGrain::getBlock(ISoundBufferCursor* cursor, SoundBlock& outBlock) const
{
	RandomGrainCursor* randomCursor = static_cast< RandomGrainCursor* >(cursor);
	return randomCursor->m_grain->getBlock(
		randomCursor->m_grainCursor,
		outBlock
	);
}

	}
}
