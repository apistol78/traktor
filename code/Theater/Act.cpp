#include "Core/Math/Const.h"
#include "Theater/Act.h"
#include "Theater/Track.h"
#include "World/Entity.h"

namespace traktor
{
	namespace theater
	{
		namespace
		{

float convolve(float x, const float* k, int32_t nk)
{
	float o = 0.0f;
	for (int32_t i = 0; i < nk; i += 3)
	{
		const float* lk = &k[i];
		o += sinf(x * lk[0] + lk[1]) * lk[2];
	}
	return o;
}

const float c_wobbleX[] =
{
	4.0f, 0.0f, 0.2f,
	5.0f, 0.4f, 0.1f,
	7.0f, 0.6f, 0.05f
};

const float c_wobbleY[] =
{
	3.0f, 0.4f, 0.2f,
	6.0f, 0.7f, 0.1f,
	8.0f, 1.1f, 0.05f
};

const float c_wobbleZ[] =
{
	2.0f, 0.8f, 0.2f,
	5.0f, 1.2f, 0.1f,
	8.0f, 1.6f, 0.05f
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.theater.Act", Act, Object)

Act::Act(float duration, bool infinite, const RefArray< const Track >& tracks)
:	m_duration(duration)
,	m_infinite(infinite)
,	m_tracks(tracks)
{
}

bool Act::update(scene::Scene* scene, float time, float deltaTime) const
{
	if (!m_infinite && time > m_duration + FUZZY_EPSILON)
		return false;

	uint32_t ntracks = uint32_t(m_tracks.size());

	TransformPath::Key key;
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
			key = path.evaluate(time + timeOffset, loopEnd, loopStart);
			transform = key.transform();
		}
		else
		{
			key = path.evaluate(clamp(time + timeOffset, 0.0f, m_duration - FUZZY_EPSILON), m_duration);
			transform = key.transform();
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
		if (lookAtEntity && lookAtEntity->getTransform(lookAtTransform))
		{
			Matrix44 m = lookAt(
				transform.translation().xyz1(),
				lookAtTransform.translation().xyz1()
			);
			transform = Transform(m.inverse());
			entity->setTransform(transform);
		}

		float wobbleMagnitude = m_tracks[i]->getWobbleMagnitude();
		float wobbleRate = m_tracks[i]->getWobbleRate();
		if (wobbleMagnitude > FUZZY_EPSILON && wobbleRate > FUZZY_EPSILON)
		{
			float dx = convolve(time * wobbleRate, c_wobbleX, sizeof_array(c_wobbleX)) * wobbleMagnitude;
			float dy = convolve(time * wobbleRate, c_wobbleY, sizeof_array(c_wobbleY)) * wobbleMagnitude;
			float dz = convolve(time * wobbleRate, c_wobbleZ, sizeof_array(c_wobbleZ)) * wobbleMagnitude;

			transform = Transform(
				transform.translation() + Vector4(dx, dy, dz),
				transform.rotation()
			);
			entity->setTransform(transform);
		}
	}

	return true;
}

	}
}
