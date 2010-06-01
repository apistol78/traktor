#include <limits>
#include "Core/Log/Log.h"
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

	//if (!m_loop && time > m_duration)
	//	return;

	TransformPath::Frame frame;
	Transform transform;

	for (RefArray< Track >::iterator i = m_tracks.begin(); i != m_tracks.end(); ++i)
	{
		Ref< world::SpatialEntity > entity = (*i)->getEntity();
		T_ASSERT (entity);

		const TransformPath& path = (*i)->getPath();

		float loopStart = (*i)->getLoopStart();
		float loopEnd = (*i)->getLoopEnd();
		float loopEase = (*i)->getLoopEase();

		if (loopStart < loopEnd && time >= loopEnd - loopEase)
		{
			float trackTime = std::fmod(time - loopStart, loopEnd - loopStart) + loopStart;

			// Calculate looped frame.
			frame = path.evaluate(trackTime, m_duration, m_loop);
			transform = Transform(frame.position, frame.orientation);

			if (trackTime >= loopEnd - loopEase)
			{
				float offset = trackTime - (loopEnd - loopEase);

				// Calculate ease into frame.
				TransformPath::Frame frame0 = path.evaluate(loopStart - loopEase + offset, m_duration, m_loop);
				Transform transform0(frame0.position, frame0.orientation);

				// Ease loop transition.
				float blend = offset / loopEase;
				transform = lerp(transform, transform0, Scalar(blend));
			}
		}
		else
		{
			// Calculate path frame.
			frame = path.evaluate(time, m_duration, m_loop);
			transform = Transform(frame.position, frame.orientation);
		}

		entity->setTransform(transform);
	}

	m_lastTime = time;
}

	}
}
