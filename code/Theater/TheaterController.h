#pragma once

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

/*! Theater scene controller.
 * \ingroup Theater
 */
class T_DLLCLASS TheaterController : public scene::ISceneController
{
	T_RTTI_CLASS;

public:
	TheaterController(const RefArray< const Act >& acts, bool repeatActs);

	virtual void update(scene::Scene* scene, float time, float deltaTime) override final;

private:
	RefArray< const Act > m_acts;
	bool m_repeatActs = false;
	bool m_hasInfinite = false;
	float m_totalDuration = 0.0f;
	float m_lastTime = -1.0f;
};

	}
}

