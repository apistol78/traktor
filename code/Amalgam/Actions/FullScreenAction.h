#ifndef traktor_amalgam_FullScreenAction_H
#define traktor_amalgam_FullScreenAction_H

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

/*! \brief Application fullscreen action.
 *
 * Applications are notified with this
 * action when Amalgam is entering or leaving fullscreen.
 */
class T_DLLCLASS FullScreenAction : public IAction
{
	T_RTTI_CLASS;

public:
	FullScreenAction(bool enteredFullScreen);

	bool enteredFullScreen() const;

private:
	bool m_enteredFullScreen;
};

	}
}

#endif	// traktor_amalgam_FullScreenAction_H
