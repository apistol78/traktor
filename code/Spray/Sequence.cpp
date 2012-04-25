#include "Spray/ITrigger.h"
#include "Spray/Sequence.h"
#include "Spray/SequenceInstance.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.Sequence", Sequence, Object)

Sequence::Sequence(const std::vector< Key >& keys)
:	m_keys(keys)
{
}

Ref< SequenceInstance > Sequence::createInstance() const
{
	std::vector< SequenceInstance::Key > keys;

	keys.resize(m_keys.size());
	for (size_t i = 0; i < m_keys.size(); ++i)
	{
		keys[i].T = m_keys[i].T;
		keys[i].trigger = m_keys[i].trigger ? m_keys[i].trigger->createInstance() : 0;
	}

	return new SequenceInstance(keys);
}

	}
}
