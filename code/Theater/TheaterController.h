#ifndef traktor_theater_TheaterController_H
#define traktor_theater_TheaterController_H

#include "Core/RefArray.h"
#include "Scene/ISceneController.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_THEATER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace theater
	{

class Track;

class T_DLLCLASS TheaterController : public scene::ISceneController
{
	T_RTTI_CLASS;

public:
	TheaterController(float duration, const RefArray< Track >& tracks);

	virtual void update(scene::Scene* scene, float time, float deltaTime);

	RefArray< Track >& getTracks() { return m_tracks; }

private:
	float m_duration;
	RefArray< Track > m_tracks;
	float m_lastTime;
};

	}
}

#endif	// traktor_theater_TheaterController_H
