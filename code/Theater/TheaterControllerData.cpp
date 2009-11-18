#include "Theater/TheaterControllerData.h"
#include "Theater/TheaterController.h"
#include "Theater/TrackData.h"
#include "Theater/Track.h"
#include "World/Entity/IEntityBuilder.h"
#include "World/Entity/SpatialEntity.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"

namespace traktor
{
	namespace theater
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.theater.TheaterControllerData", 0, TheaterControllerData, scene::ISceneControllerData)

TheaterControllerData::TheaterControllerData()
:	m_duration(0.0f)
{
}

Ref< scene::ISceneController > TheaterControllerData::createController(world::IEntityBuilder* entityBuilder, world::IEntityManager* entityManager) const
{
	RefArray< Track > tracks(m_trackData.size());
	for (size_t i = 0; i < m_trackData.size(); ++i)
	{
		Ref< world::SpatialEntity > entity = dynamic_type_cast< world::SpatialEntity* >(entityBuilder->build(m_trackData[i]->getInstance()));
		if (!entity)
			return 0;

		tracks[i] = new Track(
			entity,
			m_trackData[i]->getPath()
		);
	}
	return new TheaterController(m_duration, tracks);
}

bool TheaterControllerData::serialize(ISerializer& s)
{
	s >> Member< float >(L"duration", m_duration);
	s >> MemberRefArray< TrackData >(L"trackData", m_trackData);
	return true;
}

	}
}
