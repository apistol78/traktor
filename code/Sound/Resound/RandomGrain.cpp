#include <ctime>
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.RandomGrain", 1, RandomGrain, IGrain)

RandomGrain::RandomGrain()
:	m_random(uint32_t(clock()))
,	m_humanize(false)
,	m_last(-1)
{
}

bool RandomGrain::bind(resource::IResourceManager* resourceManager)
{
	for (RefArray< IGrain >::iterator i = m_grains.begin(); i != m_grains.end(); ++i)
	{
		if (!(*i)->bind(resourceManager))
			return false;
	}
	return true;
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

bool RandomGrain::getBlock(const ISoundMixer* mixer, ISoundBufferCursor* cursor, SoundBlock& outBlock) const
{
	RandomGrainCursor* randomCursor = static_cast< RandomGrainCursor* >(cursor);

	return randomCursor->m_grain->getBlock(
		mixer,
		randomCursor->m_grainCursor,
		outBlock
	);
}

bool RandomGrain::serialize(ISerializer& s)
{
	s >> MemberRefArray< IGrain >(L"grains", m_grains);
	if (s.getVersion() >= 1)
		s >> Member< bool >(L"humanize", m_humanize);
	return true;
}

	}
}
