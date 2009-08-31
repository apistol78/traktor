#include "Theater/TheaterController.h"
#include "Theater/Track.h"
#include "World/Entity/SpatialEntity.h"
#include "Core/Math/Const.h"

namespace traktor
{
	namespace theater
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.theater.TheaterController", TheaterController, scene::ISceneController)

TheaterController::TheaterController(float duration, const RefArray< Track >& tracks)
:	m_duration(duration)
,	m_tracks(tracks)
,	m_lastTime(0.0f)
{
}

void TheaterController::update(scene::Scene* scene, float time, float deltaTime)
{
	if (abs(m_lastTime - time) <= FUZZY_EPSILON)
		return;

	if (time > m_duration)
		return;

	for (RefArray< Track >::iterator i = m_tracks.begin(); i != m_tracks.end(); ++i)
	{
		Ref< world::SpatialEntity > entity = (*i)->getEntity();
		T_ASSERT (entity);

		const TransformPath& path = (*i)->getPath();
		TransformPath::Frame frame = path.evaluate(time);
		Matrix44 transform = translate(frame.position) * frame.orientation.toMatrix44();
		entity->setTransform(transform);
	}

	m_lastTime = time;
}

	}
}
