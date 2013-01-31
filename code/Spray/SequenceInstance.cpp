#include "Spray/ITriggerInstance.h"
#include "Spray/SequenceInstance.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.SequenceInstance", SequenceInstance, Object)

void SequenceInstance::update(Context& context, const Transform& transform, float T, bool enable)
{
	if (T < m_lastT)
	{
		m_index = 0;
		while (m_index < m_keys.size() && T >= m_keys[m_index].T)
			++m_index;

		if (m_index < m_keys.size() && m_keys[m_index].trigger)
			m_keys[m_index].trigger->perform(context, transform, enable);

		m_lastT = T;
	}

	int32_t next = m_index + 1;
	if (next < m_keys.size())
	{
		if (T >= m_keys[next].T)
		{
			if (m_keys[next].trigger)
				m_keys[next].trigger->perform(context, transform, enable);
		}
		m_index = next;
	}

	if (m_index >= 0 && m_index < m_keys.size())
	{
		if (m_keys[m_index].trigger)
			m_keys[m_index].trigger->update(context, transform, enable);
	}

	m_lastT = T;
}

SequenceInstance::SequenceInstance(const std::vector< Key >& keys)
:	m_keys(keys)
,	m_index(-1)
,	m_lastT(0.0f)
{
}

	}
}
