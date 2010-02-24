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

	virtual void setCursor(double time)
	{
		m_grainCursor->setCursor(time);
	}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.RandomGrain", 0, RandomGrain, IGrain)

RandomGrain::RandomGrain()
:	m_random(uint32_t(clock()))
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

	Ref< RandomGrainCursor > cursor = new RandomGrainCursor();
	cursor->m_grain = m_grains[index];
	cursor->m_grainCursor = m_grains[index]->createCursor();

	return cursor;
}

bool RandomGrain::getBlock(ISoundBufferCursor* cursor, SoundBlock& outBlock) const
{
	RandomGrainCursor* randomCursor = static_cast< RandomGrainCursor* >(cursor);

	return randomCursor->m_grain->getBlock(
		randomCursor->m_grainCursor,
		outBlock
	);
}

bool RandomGrain::serialize(ISerializer& s)
{
	return s >> MemberRefArray< IGrain >(L"grains", m_grains);
}

	}
}
