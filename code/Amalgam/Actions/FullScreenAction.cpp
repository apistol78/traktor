#include "Amalgam/Actions/FullScreenAction.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.FullScreenAction", FullScreenAction, IAction)

FullScreenAction::FullScreenAction(bool enteredFullScreen)
:	m_enteredFullScreen(enteredFullScreen)
{
}

bool FullScreenAction::enteredFullScreen() const
{
	return m_enteredFullScreen;
}

	}
}
