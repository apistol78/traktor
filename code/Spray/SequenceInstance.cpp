#include "Spray/Sequence.h"
#include "Spray/SequenceInstance.h"
#include "World/IEntityEventManager.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.SequenceInstance", SequenceInstance, Object)

void SequenceInstance::update(Context& context, const Transform& transform, float T, bool enable)
{
	const std::vector< Sequence::Key >& keys = m_sequence->m_keys;

	int32_t index = 0;
	for (; index < int32_t(keys.size()); ++index)
	{
		if (T >= keys[index].T)
			break;
	}

	if (index != m_index && index < int32_t(keys.size()))
	{
		context.eventManager->raise(keys[index].event, context.owner, context.owner ? transform : Transform::identity());
		m_index = index;
	}
}

SequenceInstance::SequenceInstance(const Sequence* sequence)
:	m_sequence(sequence)
,	m_index(-1)
{
}

	}
}
