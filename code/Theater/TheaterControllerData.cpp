#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Theater/TheaterController.h"
#include "Theater/TheaterControllerData.h"
#include "Theater/Track.h"
#include "Theater/TrackData.h"
#include "World/Entity.h"
#include "World/IEntityBuilder.h"

namespace traktor
{
	namespace theater
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.theater.TheaterControllerData", 1, TheaterControllerData, scene::ISceneControllerData)

TheaterControllerData::TheaterControllerData()
:	m_duration(0.0f)
,	m_loop(true)
{
}

Ref< scene::ISceneController > TheaterControllerData::createController(world::IEntityBuilder* entityBuilder, world::IEntitySchema* entitySchema) const
{
	RefArray< Track > tracks(m_trackData.size());
	for (size_t i = 0; i < m_trackData.size(); ++i)
	{
		Ref< world::Entity > entity = entityBuilder->get(m_trackData[i]->getEntityData());
		if (!entity)
			return 0;

		Ref< world::Entity > lookAtEntity = entityBuilder->get(m_trackData[i]->getLookAtEntityData());

		tracks[i] = new Track(
			entity,
			lookAtEntity,
			m_trackData[i]->getPath(),
			m_trackData[i]->getLoopStart(),
			m_trackData[i]->getLoopEnd(),
			m_trackData[i]->getLoopEase()
		);
	}
	return new TheaterController(m_duration, m_loop, tracks);
}

bool TheaterControllerData::serialize(ISerializer& s)
{
	s >> Member< float >(L"duration", m_duration);
	if (s.getVersion() >= 1)
		s >> Member< bool >(L"loop", m_loop);
	s >> MemberRefArray< TrackData >(L"trackData", m_trackData);
	return true;
}

	}
}
