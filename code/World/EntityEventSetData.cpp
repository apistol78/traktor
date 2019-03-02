#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStl.h"
#include "World/EntityEventSet.h"
#include "World/EntityEventSetData.h"
#include "World/IEntityBuilder.h"
#include "World/IEntityEventData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.EntityEventSetData", 0, EntityEventSetData, ISerializable)

Ref< EntityEventSet > EntityEventSetData::create(const IEntityBuilder* entityBuilder) const
{
	Ref< EntityEventSet > eventSet = new EntityEventSet();
	for (std::map< std::wstring, Ref< IEntityEventData > >::const_iterator i = m_eventData.begin(); i != m_eventData.end(); ++i)
	{
		Ref< world::IEntityEvent > event = entityBuilder->create(i->second);
		if (!event)
			continue;

		eventSet->m_events.insert(std::make_pair(
			i->first,
			event
		));
	}
	return eventSet;
}

void EntityEventSetData::serialize(ISerializer& s)
{
	s >> MemberStlMap<
		std::wstring,
		Ref< IEntityEventData >,
		MemberStlPair<
			std::wstring,
			Ref< IEntityEventData >,
			Member< std::wstring >,
			MemberRef< IEntityEventData >
		>
	>(L"eventData", m_eventData);
}

	}
}
