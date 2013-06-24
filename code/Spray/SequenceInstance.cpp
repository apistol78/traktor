#include "Core/Log/Log.h"
#include "Spray/ITriggerInstance.h"
#include "Spray/SequenceInstance.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.SequenceInstance", SequenceInstance, Object)

void SequenceInstance::update(Context& context, const Transform& transform, float T, bool enable)
{
	int32_t index = 0;

	for (; index < m_keys.size(); ++index)
	{
		if (T >= m_keys[index].T)
			break;
	}

	if (index < m_keys.size())
	{
		if (index != m_index)
		{
			if (m_keys[index].trigger)
				m_keys[index].trigger->perform(context, transform, enable);

			m_index = index;
		}

		if (m_keys[index].trigger)
			m_keys[index].trigger->update(context, transform, enable);
	}
}

SequenceInstance::SequenceInstance(const std::vector< Key >& keys)
:	m_keys(keys)
,	m_index(-1)
{
}

	}
}
