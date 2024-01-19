/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Sound/IAudioBuffer.h"
#include "Sound/Resound/RepeatGrain.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

struct RepeatGrainCursor : public RefCountImpl< IAudioBufferCursor >
{
	Ref< IAudioBufferCursor > m_cursor;
	uint32_t m_count;

	virtual void setParameter(handle_t id, float parameter) override final
	{
		if (m_cursor)
			m_cursor->setParameter(id, parameter);
	}

	virtual void disableRepeat() override final
	{
		m_count = ~0U;
		if (m_cursor)
			m_cursor->disableRepeat();
	}

	virtual void reset() override final
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

Ref< IAudioBufferCursor > RepeatGrain::createCursor() const
{
	if (!m_grain)
		return 0;

	Ref< RepeatGrainCursor > cursor = new RepeatGrainCursor();
	cursor->m_cursor = m_grain->createCursor();
	cursor->m_count = 0;

	return cursor->m_cursor ? cursor : 0;
}

void RepeatGrain::updateCursor(IAudioBufferCursor* cursor) const
{
	RepeatGrainCursor* repeatCursor = static_cast< RepeatGrainCursor* >(cursor);
	return m_grain->updateCursor(repeatCursor->m_cursor);
}

const IGrain* RepeatGrain::getCurrentGrain(const IAudioBufferCursor* cursor) const
{
	const RepeatGrainCursor* repeatCursor = static_cast< const RepeatGrainCursor* >(cursor);
	return m_grain->getCurrentGrain(repeatCursor->m_cursor);
}

void RepeatGrain::getActiveGrains(const IAudioBufferCursor* cursor, RefArray< const IGrain >& outActiveGrains) const
{
	const RepeatGrainCursor* repeatCursor = static_cast< const RepeatGrainCursor* >(cursor);
	T_ASSERT(repeatCursor);

	outActiveGrains.push_back(this);

	m_grain->getActiveGrains(repeatCursor->m_cursor, outActiveGrains);
}

bool RepeatGrain::getBlock(IAudioBufferCursor* cursor, const IAudioMixer* mixer, AudioBlock& outBlock) const
{
	RepeatGrainCursor* repeatCursor = static_cast< RepeatGrainCursor* >(cursor);
	if (!m_grain->getBlock(repeatCursor->m_cursor, mixer, outBlock))
	{
		if (repeatCursor->m_count == uint32_t(~0UL))
			return false;

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
