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
	TheaterController(const RefArray< const Act >& acts);

	virtual void update(scene::Scene* scene, float time, float deltaTime);

	void setCurrentAct(uint32_t current);

	uint32_t getCurrentAct() const { return m_current; }

private:
	RefArray< const Act > m_acts;
	uint32_t m_current;
	float m_lastTime;
	float m_actTime;
};

	}
}

#endif	// traktor_theater_TheaterController_H
