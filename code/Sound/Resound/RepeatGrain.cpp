#include "Sound/ISoundBuffer.h"
#include "Sound/Resound/RepeatGrain.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

struct RepeatGrainCursor : public RefCountImpl< ISoundBufferCursor >
{
	Ref< ISoundBufferCursor > m_cursor;
	uint32_t m_count;

	virtual void setParameter(handle_t id, float parameter)
	{
		if (m_cursor)
			m_cursor->setParameter(id, parameter);
	}

	virtual void reset()
	{
		if (m_cursor)
			m_cursor->reset();
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.RepeatGrain", RepeatGrain, IGrain)

RepeatGrain::RepeatGrain(
	uint32_t count,
	IGrain* grain
)
:	m_count(count)
,	m_grain(grain)
{
}

Ref< ISoundBufferCursor > RepeatGrain::createCursor() const
{
	if (!m_grain)
		return 0;

	Ref< RepeatGrainCursor > cursor = new RepeatGrainCursor();
	cursor->m_cursor = m_grain->createCursor();
	cursor->m_count = 0;

	return cursor->m_cursor ? cursor : 0;
}

void RepeatGrain::updateCursor(ISoundBufferCursor* cursor) const
{
	RepeatGrainCursor* repeatCursor = static_cast< RepeatGrainCursor* >(cursor);
	return m_grain->updateCursor(repeatCursor->m_cursor);
}

const IGrain* RepeatGrain::getCurrentGrain(const ISoundBufferCursor* cursor) const
{
	const RepeatGrainCursor* repeatCursor = static_cast< const RepeatGrainCursor* >(cursor);
	return m_grain->getCurrentGrain(repeatCursor->m_cursor);
}

bool RepeatGrain::getBlock(ISoundBufferCursor* cursor, const ISoundMixer* mixer, SoundBlock& outBlock) const
{
	RepeatGrainCursor* repeatCursor = static_cast< RepeatGrainCursor* >(cursor);
	if (!m_grain->getBlock(repeatCursor->m_cursor, mixer, outBlock))
	{
		if (m_count != 0 && ++repeatCursor->m_count >= m_count)
			return false;

		repeatCursor->m_cursor = m_grain->createCursor();
		if (!repeatCursor->m_cursor)
			return false;

		if (!m_grain->getBlock(repeatCursor->m_cursor, mixer, outBlock))
			return false;
	}
	return true;
}

	}
}
