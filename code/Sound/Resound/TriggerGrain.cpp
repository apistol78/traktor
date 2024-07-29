/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Core/Timer/Timer.h"
#include "Sound/IAudioBuffer.h"
#include "Sound/Resound/TriggerGrain.h"

namespace traktor::sound
{
	namespace
	{

struct TriggerGrainCursor : public RefCountImpl< IAudioBufferCursor >
{
	Ref< IAudioBufferCursor > m_cursor;
	handle_t m_id;
	float m_position;
	float m_rate;
	Timer m_timer;
	float m_parameter;
	bool m_active;

	virtual void setParameter(handle_t id, float parameter) override final
	{
		if (id == m_id)
		{
			const float dT = float(m_timer.getDeltaTime());
			if (m_parameter >= 0.0f)
			{
				const float rate = (parameter - m_parameter) / dT;

				if (m_rate > FUZZY_EPSILON && rate > m_rate)
					m_active = true;
				else if (m_rate < -FUZZY_EPSILON && rate < m_rate)
					m_active = true;

				if (m_position > FUZZY_EPSILON && m_parameter < m_position && parameter >= m_position)
					m_active = true;
				else if (m_position < FUZZY_EPSILON && m_parameter > -m_position && parameter <= -m_position)
					m_active = true;
			}

			m_parameter = parameter;
		}

		if (m_cursor)
			m_cursor->setParameter(id, parameter);
	}

	virtual void disableRepeat() override final
	{
		if (m_cursor)
			m_cursor->disableRepeat();
	}

	virtual void reset() override final
	{
		if (m_cursor)
			m_cursor->reset();

		m_parameter = -1.0f;
		m_active = 0.0f;
	}
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.TriggerGrain", TriggerGrain, IGrain)

TriggerGrain::TriggerGrain(
	handle_t id,
	float position,
	float rate,
	IGrain* grain
)
:	m_id(id)
,	m_position(position)
,	m_rate(rate)
,	m_grain(grain)
{
}

Ref< IAudioBufferCursor > TriggerGrain::createCursor() const
{
	if (!m_grain)
		return nullptr;

	Ref< TriggerGrainCursor > cursor = new TriggerGrainCursor();
	cursor->m_id = m_id;
	cursor->m_position = m_position;
	cursor->m_rate = m_rate;
	cursor->m_parameter = -1.0f;
	cursor->m_active = false;

	return cursor;
}

void TriggerGrain::updateCursor(IAudioBufferCursor* cursor) const
{
	TriggerGrainCursor* triggerCursor = static_cast< TriggerGrainCursor* >(cursor);
	if (triggerCursor->m_cursor)
		m_grain->updateCursor(triggerCursor->m_cursor);
}

const IGrain* TriggerGrain::getCurrentGrain(const IAudioBufferCursor* cursor) const
{
	const TriggerGrainCursor* triggerCursor = static_cast< const TriggerGrainCursor* >(cursor);
	if (triggerCursor->m_cursor)
		return m_grain->getCurrentGrain(triggerCursor->m_cursor);
	else
		return this;
}

void TriggerGrain::getActiveGrains(const IAudioBufferCursor* cursor, RefArray< const IGrain >& outActiveGrains) const
{
	const TriggerGrainCursor* triggerCursor = static_cast< const TriggerGrainCursor* >(cursor);

	outActiveGrains.push_back(this);

	if (triggerCursor && triggerCursor->m_cursor)
		m_grain->getActiveGrains(triggerCursor->m_cursor, outActiveGrains);
}

bool TriggerGrain::getBlock(IAudioBufferCursor* cursor, const IAudioMixer* mixer, AudioBlock& outBlock) const
{
	TriggerGrainCursor* triggerCursor = static_cast< TriggerGrainCursor* >(cursor);

	if (!triggerCursor->m_active)
		return true;

	if (!triggerCursor->m_cursor)
	{
		triggerCursor->m_cursor = m_grain->createCursor();
		if (!triggerCursor->m_cursor)
			return false;
	}

	if (!m_grain->getBlock(triggerCursor->m_cursor, mixer, outBlock))
	{
		triggerCursor->m_cursor = nullptr;
		triggerCursor->m_active = false;
	}

	return true;
}

}
