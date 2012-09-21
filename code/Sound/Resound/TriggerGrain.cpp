#include "Core/Math/Const.h"
#include "Core/Timer/Timer.h"
#include "Sound/ISoundBuffer.h"
#include "Sound/Resound/TriggerGrain.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

struct TriggerGrainCursor : public RefCountImpl< ISoundBufferCursor >
{
	Ref< ISoundBufferCursor > m_cursor;
	float m_position;
	float m_rate;
	Timer m_timer;
	float m_parameter;
	bool m_active;

	virtual void setParameter(float parameter)
	{
		float dT = float(m_timer.getDeltaTime());
		if (m_parameter >= 0.0f)
		{
			float rate = (parameter - m_parameter) / dT;

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

	virtual void reset()
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
	float position,
	float rate,
	IGrain* grain
)
:	m_position(position)
,	m_rate(rate)
,	m_grain(grain)
{
}

Ref< ISoundBufferCursor > TriggerGrain::createCursor() const
{
	if (!m_grain)
		return 0;

	Ref< TriggerGrainCursor > cursor = new TriggerGrainCursor();
	cursor->m_position = m_position;
	cursor->m_rate = m_rate;
	cursor->m_parameter = -1.0f;
	cursor->m_active = false;

	return cursor;
}

void TriggerGrain::updateCursor(ISoundBufferCursor* cursor) const
{
	TriggerGrainCursor* triggerCursor = static_cast< TriggerGrainCursor* >(cursor);
	if (triggerCursor->m_cursor)
		m_grain->updateCursor(triggerCursor->m_cursor);
}

bool TriggerGrain::getBlock(ISoundBufferCursor* cursor, const ISoundMixer* mixer, SoundBlock& outBlock) const
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
		triggerCursor->m_cursor = 0;
		triggerCursor->m_active = false;
	}

	return true;
}

	}
}
