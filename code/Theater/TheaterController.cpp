#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Theater/TheaterController.h"
#include "Theater/Track.h"
#include "World/Entity.h"

namespace traktor
{
	namespace theater
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.theater.TheaterController", TheaterController, scene::ISceneController)

TheaterController::TheaterController(float duration, const RefArray< Track >& tracks)
:	m_duration(duration)
,	m_tracks(tracks)
,	m_lastTime(-std::numeric_limits< float >::max())
{
}

void TheaterController::update(scene::Scene* scene, float time, float deltaTime)
{
	if (abs(m_lastTime - time) <= FUZZY_EPSILON)
		return;

	uint32_t ntracks = m_tracks.size();

	TransformPath::Frame frame;
	Transform lookAtTransform;
	Transform transform;

	// Calculate transforms.
	for (uint32_t i = 0; i < ntracks; ++i)
	{
		Ref< world::Entity > entity = m_tracks[i]->getEntity();
		T_ASSERT (entity);

		const TransformPath& path = m_tracks[i]->getPath();

		float loopStart = m_tracks[i]->getLoopStart();
		float loopEnd = m_tracks[i]->getLoopEnd();
		float timeOffset = m_tracks[i]->getTimeOffset();

		if (loopStart + FUZZY_EPSILON < loopEnd)
		{
			frame = path.evaluate(time + timeOffset, loopEnd, loopStart);
			transform = frame.transform();
		}
		else
		{
			frame = path.evaluate(time + timeOffset, m_duration);
			transform = frame.transform();
		}

		entity->setTransform(transform);
	}

	// Fixup orientation of "looking" entities.
	for (uint32_t i = 0; i < ntracks; ++i)
	{
		Ref< world::Entity > entity = m_tracks[i]->getEntity();
		T_ASSERT (entity);

		if (!entity->getTransform(transform))
			continue;

		Ref< world::Entity > lookAtEntity = m_tracks[i]->getLookAtEntity();
		if (!lookAtEntity || !lookAtEntity->getTransform(lookAtTransform))
			continue;
		
		Matrix44 m = lookAt(
			transform.translation().xyz1(),
			lookAtTransform.translation().xyz1()
		);
		transform = Transform(m.inverse());

		entity->setTransform(transform);
	}

	m_lastTime = time;
}

	}
}
