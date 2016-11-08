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

class Act;

class T_DLLCLASS TheaterController : public scene::ISceneController
{
	T_RTTI_CLASS;

public:
	TheaterController(const RefArray< const Act >& acts, bool repeatActs);

	virtual void update(scene::Scene* scene, float time, float deltaTime) T_OVERRIDE T_FINAL;

private:
	RefArray< const Act > m_acts;
	bool m_repeatActs;
	float m_totalDuration;
	float m_lastTime;
};

	}
}

#endif	// traktor_theater_TheaterController_H
