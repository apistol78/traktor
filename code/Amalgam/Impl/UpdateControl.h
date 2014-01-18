#ifndef traktor_amalgam_UpdateControl_H
#define traktor_amalgam_UpdateControl_H

#include "Amalgam/IUpdateControl.h"

namespace traktor
{
	namespace amalgam
	{

/*! \brief Update control. */
class UpdateControl : public IUpdateControl
{
	T_RTTI_CLASS;

public:
	bool m_pause;
	float m_timeScale;

	UpdateControl()
	:	m_pause(false)
	,	m_timeScale(1.0f)
	{
	}

	virtual void setPause(bool pause) { m_pause = pause; }

	virtual bool getPause() const { return m_pause; }

	virtual void setTimeScale(float timeScale) { m_timeScale = timeScale; }

	virtual float getTimeScale() const { return m_timeScale; }
};

	}
}

#endif	// traktor_amalgam_UpdateControl_H
