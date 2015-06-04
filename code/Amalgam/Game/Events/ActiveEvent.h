#ifndef traktor_amalgam_ActiveEvent_H
#define traktor_amalgam_ActiveEvent_H

#include "Amalgam/Game/IEvent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_GAME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

/*! \brief Application active events.
 * \ingroup Amalgam
 *
 * Applications are notified with this
 * event when Amalgam is becoming activated or deactivated.
 */
class T_DLLCLASS ActiveEvent : public IEvent
{
	T_RTTI_CLASS;

public:
	ActiveEvent(bool activated);

	bool becameActivated() const;

private:
	bool m_activated;
};

	}
}

#endif	// traktor_amalgam_ActiveEvent_H
