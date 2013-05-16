#ifndef traktor_theater_Act_H
#define traktor_theater_Act_H

#include "Core/Object.h"
#include "Core/RefArray.h"

namespace traktor
{
	namespace scene
	{

class Scene;

	}

	namespace theater
	{

class Track;

class Act : public Object
{
	T_RTTI_CLASS;

public:
	Act(float duration, bool infinite, const RefArray< const Track >& tracks);

	bool update(scene::Scene* scene, float time, float deltaTime) const;

private:
	float m_duration;
	bool m_infinite;
	RefArray< const Track > m_tracks;
};

	}
}

#endif	// traktor_theater_Act_H
