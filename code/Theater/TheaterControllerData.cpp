#include "Theater/TheaterControllerData.h"
#include "Theater/TheaterController.h"
#include "Theater/TrackData.h"
#include "Theater/Track.h"
#include "World/Entity/IEntityBuilder.h"
#include "World/Entity/SpatialEntity.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace theater
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.theater.TheaterControllerData", TheaterControllerData, scene::ISceneControllerData)

TheaterControllerData::TheaterControllerData()
:	m_duration(0.0f)
{
}

scene::ISceneController* TheaterControllerData::createController(world::IEntityBuilder* entityBuilder, world::IEntityManager* entityManager) const
{
	RefArray< Track > tracks(m_trackData.size());
	for (size_t i = 0; i < m_trackData.size(); ++i)
	{
		Ref< world::SpatialEntity > entity = dynamic_type_cast< world::SpatialEntity* >(entityBuilder->build(m_trackData[i]->getInstance()));
		if (!entity)
			return 0;

		tracks[i] = gc_new< Track >(
			entity,
			cref(m_trackData[i]->getPath())
		);
	}
	return gc_new< TheaterController >(m_duration, cref(tracks));
}

bool TheaterControllerData::serialize(Serializer& s)
{
	s >> Member< float >(L"duration", m_duration);
	s >> MemberRefArray< TrackData >(L"trackData", m_trackData);
	return true;
}

	}
}
