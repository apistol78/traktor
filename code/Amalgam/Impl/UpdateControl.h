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

	UpdateControl()
	:	m_pause(false)
	{
	}

	virtual void setPause(bool pause) { m_pause = pause; }

	virtual bool getPause() const { return m_pause; }
};

	}
}

#endif	// traktor_amalgam_UpdateControl_H
