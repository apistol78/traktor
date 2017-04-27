/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/Const.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Theater/Act.h"
#include "Theater/ActData.h"
#include "Theater/Track.h"
#include "Theater/TrackData.h"
#include "World/Entity.h"

namespace traktor
{
	namespace theater
	{
		namespace
		{

world::Entity* findEntityDataProduct(const std::map< const world::EntityData*, Ref< world::Entity > >& entityProducts, const world::EntityData* entityData)
{
	std::map< const world::EntityData*, Ref< world::Entity > >::const_iterator i = entityProducts.find(entityData);
	return i != entityProducts.end() ? i->second : 0;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.theater.ActData", 0, ActData, ISerializable)

ActData::ActData()
:	m_duration(5.0f)
{
}

Ref< Act > ActData::createInstance(const std::map< const world::EntityData*, Ref< world::Entity > >& entityProducts) const
{
	bool infinite = false;

	RefArray< const Track > tracks(m_tracks.size());
	for (size_t i = 0; i < m_tracks.size(); ++i)
	{
		Ref< world::Entity > entity = findEntityDataProduct(entityProducts, m_tracks[i]->getEntityData());
		if (!entity)
			return 0;

		Ref< world::Entity > lookAtEntity = findEntityDataProduct(entityProducts, m_tracks[i]->getLookAtEntityData());

		tracks[i] = new Track(
			entity,
			lookAtEntity,
			m_tracks[i]->getPath(),
			m_tracks[i]->getLoopStart(),
			m_tracks[i]->getLoopEnd(),
			m_tracks[i]->getTimeOffset(),
			m_tracks[i]->getWobbleMagnitude(),
			m_tracks[i]->getWobbleRate()
		);

		if (m_tracks[i]->getLoopStart() < m_tracks[i]->getLoopEnd() - FUZZY_EPSILON)
			infinite |= true;
	}

	return new Act(m_duration, infinite, tracks);
}

void ActData::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> Member< float >(L"duration", m_duration);
	s >> MemberRefArray< TrackData >(L"tracks", m_tracks);
}

	}
}
