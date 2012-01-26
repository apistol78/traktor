#include "Spray/ITriggerInstance.h"
#include "Spray/SequenceInstance.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.SequenceInstance", SequenceInstance, Object)

void SequenceInstance::update(Context& context, const Transform& transform, float T)
{
	if (T < m_lastT)
	{
		m_index = 0;
		while (m_index < m_keys.size() && T >= m_keys[m_index].T)
			++m_index;

		m_lastT = T;
	}

	if (m_index >= m_keys.size() || T < m_keys[m_index].T)
		return;

	if (m_keys[m_index].trigger)
		m_keys[m_index].trigger->perform(context, transform);

	++m_index;
	m_lastT = T;
}

SequenceInstance::SequenceInstance(const std::vector< Key >& keys)
:	m_keys(keys)
,	m_index(0)
,	m_lastT(0.0f)
{
}

	}
}
