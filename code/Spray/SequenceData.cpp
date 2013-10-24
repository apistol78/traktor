#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStl.h"
#include "Spray/Sequence.h"
#include "Spray/SequenceData.h"
#include "World/IEntityBuilder.h"
#include "World/IEntityEventData.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.SequenceData", 0, SequenceData, ISerializable)

Ref< Sequence > SequenceData::createSequence(const world::IEntityBuilder* entityBuilder) const
{
	std::vector< Sequence::Key > keys;

	keys.resize(m_keys.size());
	for (size_t i = 0; i < m_keys.size(); ++i)
	{
		keys[i].T = m_keys[i].T;
		keys[i].event = entityBuilder->create(m_keys[i].event);
	}

	return new Sequence(keys);
}

void SequenceData::serialize(ISerializer& s)
{
	s >> MemberStlVector< Key, MemberComposite< Key > >(L"keys", m_keys);
}

void SequenceData::Key::serialize(ISerializer& s)
{
	s >> Member< float >(L"T", T);
	s >> MemberRef< world::IEntityEventData >(L"event", event);
}

	}
}
