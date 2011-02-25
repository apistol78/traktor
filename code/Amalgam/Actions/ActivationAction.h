#ifndef traktor_amalgam_ActivationAction_H
#define traktor_amalgam_ActivationAction_H

#include "Amalgam/IAction.h"

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

/*! \brief Application activation action.
 * \ingroup Amalgam
 *
 * Applications are notified with this
 * action when Amalgam is becoming activated or deactivated.
 */
class T_DLLCLASS ActivationAction : public IAction
{
	T_RTTI_CLASS;

public:
	ActivationAction(bool activated);

	bool becameActivated() const;

private:
	bool m_activated;
};

	}
}

#endif	// traktor_amalgam_ActivationAction_H
