#include "Amalgam/Impl/ErrorState.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.ErrorState", ErrorState, IState)

ErrorState::ErrorState(Environment* environment)
:	m_environment(environment)
{
}

void ErrorState::enter()
{
}

void ErrorState::leave()
{
}

IState::UpdateResult ErrorState::update(IStateManager* stateManager, IUpdateControl& control, const IUpdateInfo& info)
{
	return UrExit;
}

IState::BuildResult ErrorState::build(uint32_t frame, const IUpdateInfo& info)
{
	return BrNothing;
}

bool ErrorState::render(uint32_t frame, render::EyeType eye, const IUpdateInfo& info)
{
	return false;
}

	}
}
