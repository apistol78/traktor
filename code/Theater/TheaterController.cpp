#include <limits>
#include "Core/Math/Const.h"
#include "Theater/TheaterController.h"
#include "Theater/Track.h"
#include "World/Entity/SpatialEntity.h"

namespace traktor
{
	namespace theater
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.theater.TheaterController", TheaterController, scene::ISceneController)

TheaterController::TheaterController(float duration, bool loop, const RefArray< Track >& tracks)
:	m_duration(duration)
,	m_loop(loop)
,	m_tracks(tracks)
,	m_lastTime(-std::numeric_limits< float >::max())
{
}

void TheaterController::update(scene::Scene* scene, float time, float deltaTime)
{
	if (abs(m_lastTime - time) <= FUZZY_EPSILON)
		return;

	if (!m_loop && time > m_duration)
		return;

	for (RefArray< Track >::iterator i = m_tracks.begin(); i != m_tracks.end(); ++i)
	{
		Ref< world::SpatialEntity > entity = (*i)->getEntity();
		T_ASSERT (entity);

		const TransformPath& path = (*i)->getPath();
		TransformPath::Frame frame = path.evaluate(time, m_duration, m_loop);
		Transform transform(frame.position, frame.orientation);
		entity->setTransform(transform);
	}

	m_lastTime = time;
}

	}
}
