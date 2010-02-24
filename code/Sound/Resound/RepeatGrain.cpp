#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
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

	virtual void setCursor(double time)
	{
		m_cursor->setCursor(time);
	}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.RepeatGrain", 0, RepeatGrain, IGrain)

RepeatGrain::RepeatGrain()
:	m_count(1)
{
}

bool RepeatGrain::bind(resource::IResourceManager* resourceManager)
{
	return m_grain ? m_grain->bind(resourceManager) : true;
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

bool RepeatGrain::getBlock(ISoundBufferCursor* cursor, SoundBlock& outBlock) const
{
	RepeatGrainCursor* loopCursor = static_cast< RepeatGrainCursor* >(cursor);

	if (!m_grain->getBlock(loopCursor->m_cursor, outBlock))
	{
		if (m_count != 0 && ++loopCursor->m_count >= m_count)
			return false;

		loopCursor->m_cursor = m_grain->createCursor();
		if (!loopCursor->m_cursor)
			return false;

		if (!m_grain->getBlock(loopCursor->m_cursor, outBlock))
			return false;
	}

	return true;
}

bool RepeatGrain::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"count", m_count);
	s >> MemberRef< IGrain >(L"grain", m_grain);
	return true;
}

	}
}
