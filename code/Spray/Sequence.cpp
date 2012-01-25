#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStl.h"
#include "Spray/ITrigger.h"
#include "Spray/Sequence.h"
#include "Spray/SequenceInstance.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.Sequence", 0, Sequence, ISerializable)

bool Sequence::bind(resource::IResourceManager* resourceManager)
{
	for (std::vector< Key >::iterator i = m_keys.begin(); i != m_keys.end(); ++i)
	{
		if (!i->trigger)
			continue;

		if (!i->trigger->bind(resourceManager))
			return false;
	}
	return true;
}

Ref< SequenceInstance > Sequence::createInstance()
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

bool Sequence::serialize(ISerializer& s)
{
	return s >> MemberStlVector< Key, MemberComposite< Key > >(L"keys", m_keys);
}

bool Sequence::Key::serialize(ISerializer& s)
{
	s >> Member< float >(L"T", T);
	s >> MemberRef< ITrigger >(L"trigger", trigger);
	return true;
}

	}
}
