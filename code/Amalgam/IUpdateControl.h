#ifndef traktor_amalgam_IUpdateControl_H
#define traktor_amalgam_IUpdateControl_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

/*! \brief Update control.
 * \ingroup Amalgam
 *
 * Application update control interface, as
 * applications can become paused or similar
 * either internally, from application itself,
 * or externally, window minimized, this
 * interface provide a two-way method of
 * handling update scenarios.
 */
class T_DLLCLASS IUpdateControl : public Object
{
	T_RTTI_CLASS;

public:
	virtual void setPause(bool pause) = 0;

	virtual bool getPause() const = 0;

	virtual void setTimeScale(float timeScale) = 0;

	virtual float getTimeScale() const = 0;

	virtual void setSimulationFrequency(float simulationFrequency) = 0;

	virtual float getSimulationFrequency() const = 0;
};

	}
}

#endif	// traktor_amalgam_IUpdateControl_H
